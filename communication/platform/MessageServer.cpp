#include "MessageServer.h"
#include "Exception.h"


#include <sys/select.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

#include <string>
#include <algorithm>

using namespace std;

const unsigned int MessageServer::MAX_MSGSIZE_INCOMING = 2000;
const int MessageServer::READ_TIMEOUT = 60; // in seconds
const int MessageServer::BUFLENGTH = 400;
const int MessageServer::MAXIMAL_TIMEOUT = 2*60-10;
const vector<MessageHandler::byte> MessageServer::FAIL  = vector<MessageHandler::byte>(1, '0');
const vector<MessageHandler::byte> MessageServer::OK    = vector<MessageHandler::byte>(1, '1');
const vector<MessageHandler::byte> MessageServer::REQUEST_PENDING = vector<MessageHandler::byte>(1, '3');


/*
 * Some code shamelessly copied from different online sources, such as
 * http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 * http://www.lowtek.com/sockets/select.html
 */


MessageServer::MessageServer(configlib::configfile& config) :
	Log("MessageServer"), handler(NULL),
	port(config, "MessageServer", "port", "-p", 2222),
	watchdog(SoftwareWatchdog::ID_MESSAGESERVER, false)
{
}

MessageServer::~MessageServer()
{
	
}

void MessageServer::set_handler(MessageHandler *_handler)
{
	handler = _handler;
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
		throw(Exception("while getting socket flags", errno ));
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(socket,F_SETFL,opts) < 0) {
		throw(Exception("while setting socket flags", errno ));
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
 * \exception std::string
 */
int MessageServer::run()
{
	int listening_socket = -1, new_socket;
	socklen_t clilen;
	struct sockaddr_in server_address, client_address;
	struct timespec timeout;
	timeout.tv_nsec = 0;
	timeout.tv_sec = MAXIMAL_TIMEOUT;
	fd_set read_sockets;
	sigset_t all_signals, listening_signals;
	sigfillset(&all_signals);
	sigfillset(&listening_signals);
	sigdelset(&listening_signals, SIGUSR1);
	// block all signals. only listen to SIGUSR1 in select()
	pthread_sigmask(SIG_SETMASK, &all_signals, NULL);

	assert(handler != NULL);
	info << "starting" << endl;
	
	done = false;
	try {
		// create socket
		listening_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (listening_socket < 0) {
			throw(Exception("opening socket", errno));
		}
		set_non_blocking(listening_socket);
		// bind to address
		bzero((char *) &server_address, sizeof(server_address));
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = INADDR_ANY;
		server_address.sin_port = htons((int)port);
		if(bind(listening_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
			throw(Exception("binding to address", errno));
		}
		// listen
		listen(listening_socket,5);
		info << "listening on port " << (int)port << endl;
		clilen = sizeof(client_address);
		
		while(1) {
			FD_ZERO(&read_sockets);
			FD_SET(listening_socket, &read_sockets);
			/* to avoid race conditions:
			 *  - disable termination signals
			 *  - check if signal has arrived
			 *  - call pselect, which returns on the termination signals
			 *  - reenable termination signals
			 */
			/// @todo see what happens, when we stop the thread in between select calls
			watchdog.serve();
			int n = pselect(listening_socket+1, &read_sockets, NULL, NULL, &timeout, &listening_signals);
			watchdog.serve();
			if(n == 0) {
				// timeout only, nothing on the wire
				continue;
			}
			if(n < 0) {
				if(done) {
					break;
				}
				// some error...
				throw(Exception("on select on listening socket", errno));
			}
			// will not block, according to select()
			new_socket = accept(listening_socket, (struct sockaddr *) &client_address, &clilen);
			if(new_socket == EWOULDBLOCK) {
				// ignore spurious return of accept()
				continue;
			}
			if(new_socket < 0) {
				throw(Exception("on accept", errno));
			}
			handle_connection(new_socket, &client_address.sin_addr);
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
	info << "done" << endl;
	return 0;
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
	MessageHandler::byte buffer[BUFLENGTH];
	int n;
	char s[INET_ADDRSTRLEN];
	vector<MessageHandler::byte> received;
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
				throw(Exception("while waiting for data", errno));
			}
			if(n == 0) {
				// timed out
				throw(TimeoutException("while waiting for data"));
			}
			assert(n == 1);
			n = read(socket, buffer, BUFLENGTH);
			if(n == EAGAIN) {
				// ignore spurious return of select()
				continue;
			}
			if(n < 0) {
				throw(Exception("reading from socket", errno));
			}
			if(n == 0) {
				// no more data available
				break;
			}
			if(received.size() + n > MAX_MSGSIZE_INCOMING) {
				throw(Exception("while receiving data: incoming message too big"));
			}
			received.reserve(received.size() + n);
			received.insert(received.end(), buffer, buffer + n);
//			received.append(buffer, n);
		} while(1);
		
		info << "finished reading from socket" << endl;
		const vector<MessageHandler::byte> answer = handle_data(received);
		unsigned int sent = 0;
		///@todo write timeout
		do {
			int i;
			for(i = 0; i < BUFLENGTH && i + sent < answer.size(); i++) {
				buffer[i] = answer[i + sent];
			}
			int n = write(socket, buffer, i);
			if (n < 0) {
				throw(Exception("writing to socket", errno));
			}
			sent += n;
		} while(sent < answer.size());		
	}
	catch(TimeoutException& e) {
		error << "Timeout while reading from socket" << endl;	
	}
	catch(Exception& e) {
		error << "Error in client connection: " << e.what() << endl;
		error << "Dropping connection." << endl;
	}
	catch(...) {
		error << "Unknown error in client connection" << endl;
		error << "Dropping connection." << endl;
	}
//	shutdown(socket, 2);
	close(socket);
	info << "closed connection" << endl;
}

/**
 * Handles the data received in one connection according to the communication protocol.
 * 
 * \param data	data received from the client
 * \return	the data to be sent to the client.
 * \exception std::string
 */
std::vector<MessageHandler::byte> MessageServer::handle_data(std::vector<MessageHandler::byte> data)
{
	vector<MessageHandler::byte>::iterator i = data.begin();
	vector<MessageHandler::byte> state, answer;
	MessageHandler::RequestStatus status;
	switch(*i++) {
		case 's':
			status = handler->send_status(string(i, data.end()));
			info << "received status message: ";
			for(; i != data.end(); i++) {
				info << *i;
			}
			info << endl;
			break;
		case 'r': {
			std::string message;
			status = handler->receive_message(message);
			answer = std::vector<MessageHandler::byte>(message.begin(), message.end());
			info << "received read for command message: ";
			info << endl;
			break;
		}
		default:
			throw(Exception("unknown command"));
	}
	switch(status) {
		case MessageHandler::FAIL:
			state = FAIL;
		case MessageHandler::OK:
			state = OK;
		default:
			throw(Exception("unknown return value for request"));
	}
	state.insert(state.end(), answer.begin(), answer.end());
	return state;
}


