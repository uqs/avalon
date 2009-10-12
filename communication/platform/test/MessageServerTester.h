#ifndef MESSAGEHANDLERMOCK_H_
#define MESSAGEHANDLERMOCK_H_

#include "MessageHandler.h"
#include "Tester.h"

#include <string>

class MessageServerTester : public MessageHandler, public Tester
{
	std::string status;
	std::string request;
	std::string forecast;
	bool polled;
	bool read;
	bool wait;
	MessageHandler::RequestStatus answer;
	
	int sock;
	static const int PORT;

public:
	MessageServerTester();
	virtual ~MessageServerTester();

	virtual bool test();
	void connect();
	std::string transmit(const std::string &data);
	void send(const std::string &data);
	void shutdown();
	std::string receive();
	void close();

	virtual RequestStatus send_status(const std::string &s);
	virtual RequestStatus receive_message(std::string &message);
	virtual RequestStatus read_forecast(std::vector<byte> &forecast);
	
};

#endif /*MESSAGEHANDLERMOCK_H_*/
