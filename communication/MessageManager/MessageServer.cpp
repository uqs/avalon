#include "MessageServer.h"

#include <sys/select.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <string>


using namespace std;

const unsigned int MessageServer::MAX_MSGSIZE_INCOMING = 2000;
const int  MessageServer::READ_TIMEOUT = 60; // in seconds
const int  MessageServer::BUFLENGTH = 256;
const char MessageServer::SUCCESS[] = "ok";
const char MessageServer::FAIL[] = "fail";

bool MessageServer::done = false;


/*
 * Some code shamelessly copied from different online sources, such as
 * http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 * http://www.lowtek.com/sockets/select.html
 */



MessageServer::MessageServer(int port) : portno(port), info(cout.rdbuf()), error(cerr.rdbuf())
{
}

MessageServer::~MessageServer()
{
	
}

void MessageServer::set_logfile(streambuf *logfile) {
	info.rdbuf(logfile);
	error.rdbuf(logfile);
}


void MessageServer::log(std::string message, LogLevel severity)
{
	switch(severity) {
		case Normal:
			info << message << endl;
			break;
		case Error:
			error << message << endl;
			break;
	}			
}

void MessageServer::signal_stop() {
	MessageServer::done = true;	
}

/**
 * Switches a socket to nonblocking mode. read() will return EAGAIN if no data is present
 * and select() will return with EWOULDBLOCK if no connection is pending.
 * 
 * \exception std::string if anything goes wrong
 */
void MessageServer::set_non_blocking(int socket) {
	int opts;

	opts = fcntl(socket,F_GETFL);
	if (opts < 0) {
		throw(string("while getting socket flags: ") + strerror(errno) );
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(socket,F_SETFL,opts) < 0) {
		throw(string("while setting socket flags: ") + strerror(errno) );
	}
}

/**
 * Run the server!
 * The server is strictly single-threaded and only handles one connection at once.
 * If additional connections are opened while one is already established, they are
 * queued to be accepted.
 * 
 * Open question: should the server restart by itself if anything goes wrong or throw
 * an exception?
 * 
 * \param fork whether the server should fork into background and act as a daemon.
 * \exception std::string
 */
void MessageServer::run()
{
	int listening_socket = -1, new_socket;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	fd_set read_sockets;
	sigset_t term_signals, normal_signals;
	sigemptyset(&term_signals);
	sigaddset(&term_signals, SIGINT);
	sigaddset(&term_signals, SIGTERM);

	info << "MessageManager starting" << endl;
	
	done = false;
	try {
		// create socket
		listening_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (listening_socket < 0) {
			throw(string("opening socket: ") + strerror(errno));
		}
		set_non_blocking(listening_socket);
		// bind to address
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		if(bind(listening_socket, (struct sockaddr *) &serv_addr,	sizeof(serv_addr)) < 0) {
			throw(string("binding to address: ") + strerror(errno));
		}
		// listen
		listen(listening_socket,5);
		info << "listening on port " << portno << endl;
		clilen = sizeof(cli_addr);
		
		while(1) {
			FD_ZERO(&read_sockets);
			FD_SET(listening_socket, &read_sockets);
			/* to avoid race conditions:
			 *  - disable termination signals
			 *  - check if signal has arrived
			 *  - call pselect, which returns on the termination signals
			 *  - reenable termination signals
			 */
			sigprocmask(SIG_BLOCK, &term_signals, &normal_signals);
			if(done) {
				break;
			}
			int n = pselect(listening_socket+1, &read_sockets, NULL, NULL, NULL, &normal_signals);
			sigprocmask(SIG_UNBLOCK, &term_signals, NULL);
			if(n < 0) {
				if(done) {
					break;
				}
				// some error...
				throw(string("on select on listening socket: ") + strerror(errno));
			}				
			// will not block, according to select()
			new_socket = accept(listening_socket, (struct sockaddr *) &cli_addr, &clilen);
			if(new_socket == EWOULDBLOCK) {
				// ignore spurious return of accept()
				continue;
			}
			if(new_socket < 0) {
				throw(string("on accept: ") + strerror(errno));
			}
			handle_connection(new_socket, &cli_addr.sin_addr);
		}
	}
	catch(...) {
		// error handling
		if(listening_socket >= 0) {
			close(listening_socket);				
		}
		throw;
	}
	close(listening_socket);
	info << "MessageManager done" << endl;
}

/**
 * Handles one client request. Reads all data until the FIN flag is set (client finnishes transmission).
 * Then calls handle_data() on the received data. Lastly sends the data returned by handle_data() to the
 * client.
 * 
 * \param socket Socket of client connection.
 * \param addr   Address structure describing the client
 */
void MessageServer::handle_connection(int socket, struct in_addr *addr)
{
	char buffer[BUFLENGTH];
	int n;
	char s[INET_ADDRSTRLEN];
	string received;
	fd_set read_sockets;
	struct timeval timeout;
	
	timeout.tv_usec = 0;
	timeout.tv_sec = READ_TIMEOUT;
	
	set_non_blocking(socket);

	try {
		if(inet_ntop(AF_INET, addr, s, sizeof s) == 0) {
			strncpy(s, "unknown", sizeof s);
		}
		info << "acceped new connection from " << s << endl;
		do {
			FD_ZERO(&read_sockets);
			FD_SET(socket,&read_sockets);
			n = select(socket+1, &read_sockets, NULL, NULL, &timeout);
			if(n < 0) {
				if(done) {
					info << "received signal for termination, waiting for connection to end" << endl;
					continue;
				}
				throw(string("while waiting for data: ") + strerror(errno));
			}
			if(n == 0) {
				// timed out
				throw(TimeoutException());
			}
			assert(n == 1);
			n = read(socket, buffer, BUFLENGTH);
			if(n == EAGAIN) {
				// ignore spurious return of select()
				continue;
			}
			if(n < 0) {
				throw(string("reading from socket: ") + strerror(errno));
			}
			if(n == 0) {
				// no more data available
				break;
			}
			if(received.size() + n > MAX_MSGSIZE_INCOMING) {
				throw(string("while receiving data: incoming message too big"));
			}
			received.reserve(received.size() + n);
			received.insert(received.end(), buffer, buffer + n);
		} while(1);
		
		info << "finished reading from socket" << endl;
		const string answer = handle_data(received);
		unsigned int sent = 0;
		do {
			
			int n = write(socket, answer.c_str() + sent, answer.size() - sent);
			if (n < 0) {
				throw(string("writing to socket: ") + strerror(errno));
			}
			sent += n;
		} while(sent < answer.size());		
	}
	catch(TimeoutException e) {
		error << "Timeout while reading from socket" << endl;	
	}
	catch(string msg) {
		error << "Error in client connection: " << msg << endl;
		error << "Dropping connection." << endl;
	}
	catch(...) {
		error << "Unknown error in client connection" << endl;
		error << "Dropping connection." << endl;
	}
	shutdown(socket, 2);
	close(socket);
	info << "closed connection" << endl << endl;
}

/**
 * Handles the data received in one connection according to the communication protocol.
 * 
 * \param data data received from the client
 * \return    the data to be sent to the client.
 * \exception std::string
 */
string MessageServer::handle_data(const string data)
{
	string::const_iterator i = data.begin();
	switch(*i++) {
		case 's':
			info << "received status message: "; 
			for(; i != data.end(); i++) {
				info << *i;
			}
			info << endl;
			return SUCCESS;
			break;
		case 'r':
			info << "received request for weather forecast: "; 
			for(; i != data.end(); i++) {
				info << *i;
			}
			info << endl;
			return SUCCESS;
			break;
		case 'p':
			info << "received poll for weather forecast" << endl;
			return SUCCESS;
			break;
		case 'f':
			info << "received read for weather forecast" << endl;
			return "schwache brise aus osten";
			break;
		default:
			throw(string("unknown command"));
	}	
}


