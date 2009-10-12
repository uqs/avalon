#ifndef SERVER_H_
#define SERVER_H_

#include "logstream.h"

#include <string>
#include <exception>
#include <ostream>
#include <fstream>

class Server
{
	// data types
protected:	
	class TimeoutException : public std::exception {
		
	};
	
	enum LogLevel {
		Normal,
		Error
	};



	// data members
protected:
	int portno;
	logstream info;
	logstream error; 
	
	// static
	static bool done;

	const static int BUFLENGTH;
	const static unsigned int MAX_MSGSIZE_INCOMING;
	const static char SUCCESS[];
	const static char FAIL[];
	const static int  READ_TIMEOUT;

	// methods
protected:
	void handle_connection(int socket, struct in_addr *addr);
	std::string handle_data(const std::string data);

	void set_non_blocking(int socket);
	void log(std::string message, LogLevel severity = Normal);

public:
	Server(int port);
	void set_logfile(std::streambuf *logfile);
	virtual ~Server();
	void run();
	
	static void signal_stop();
	
	// friends
protected:
	friend void signal_handler(int signal);
	friend void child_handler(int signum);
//	friend Server::logstream& operator<<(Server::logstream& log, std::ostream& (*fn)(std::ostream&));
	
};


#endif /*SERVER_H_*/
