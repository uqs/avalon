#ifndef SERVER_H_
#define SERVER_H_

#include "MessageHandler.h"
#include "Thread.h"
#include "Log.h"
#include "SoftwareWatchdog.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"

#include <vector>
#include <exception>
#include <ostream>
#include <fstream>

class MessageServer : public Thread, public Log
{

	// data members
private:
	MessageHandler* handler;
	configlib::configitem<int> port;
	SoftwareWatchdog watchdog;
	// static

	static const int BUFLENGTH;
	static const unsigned int MAX_MSGSIZE_INCOMING;
	static const int READ_TIMEOUT;
	static const int MAXIMAL_TIMEOUT;
	static const std::vector<MessageHandler::byte> OK;
	static const std::vector<MessageHandler::byte> FAIL;
	static const std::vector<MessageHandler::byte> REQUEST_PENDING;

	// methods
private:
	void handle_connection(int socket, struct in_addr *addr);
	std::vector<MessageHandler::byte> handle_data(const std::vector<MessageHandler::byte> data);

	void set_non_blocking(int socket);

public:
	MessageServer(configlib::configfile& config);
	virtual ~MessageServer();
	int run();
	void set_handler(MessageHandler *_handler);

};


#endif /*SERVER_H_*/
