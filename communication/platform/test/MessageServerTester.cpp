#include "MessageServerTester.h"
#include "MessageServer.h"
#include "Exception.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include <iostream>

using namespace std;

const int MessageServerTester::PORT = 2222;

MessageServerTester::MessageServerTester()
{
}

MessageServerTester::~MessageServerTester()
{
}

bool MessageServerTester::test()
{
	string sent, received;
	bool passed = true;
	wait = false;
	
	filebuf logfile;
	logfile.open("mstest.log", ios::out|ios::trunc);
	if(!logfile.is_open()) {
		throw(Exception("error opening logfile"));
	}
	cout << "server logfile is in ./mstest.log" << endl;
	cout << "config file read from /dev/null" << endl;
	configlib::configfile config("/dev/null");
	
	MessageServer server(config);
	server.set_handler(this);
	server.set_logfile(&logfile);
	server.start();
	
	// ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	
	sleep(1);
	
	cout << "testing send_status(): ";
	answer = OK;
	sent = "alles klar hier";
	received = transmit("s" + sent);
	sleep(1);
	if(sent != status) {
		cout << "failed" << endl;
		cout << "sent: " << sent << endl;
		cout << "received: " << status << endl;
		passed = false;
	}
	else if(received != "1") {
		cout << "failed" << endl;
		cout << "expected: 1" << endl;
		cout << "received: " << received << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}

	cout << "testing send_status(): ";
	answer = FAIL;
	sent = "nichts geht mehr!";
	received = transmit("s" + sent);
	sleep(1);
	if(sent != status) {
		cout << "failed" << endl;
		cout << "sent: " << sent << endl;
		cout << "received: " << status << endl;
		passed = false;
	}
	else if(received != "0") {
		cout << "failed" << endl;
		cout << "expected: 0" << endl;
		cout << "received: " << received << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}
	
	cout << "testing read_forecast(): ";
	answer = OK;
	forecast = "leichte Brise aus Osten";
	received = transmit("f");
	sleep(1);
	if(received[0] != '1') {
		cout << "failed" << endl;
		cout << "expected: 1" << endl;
		cout << "received: " << received[0] << endl;
		passed = false;
	}
	else if(received.erase(0, 1) != forecast) {
		cout << "failed" << endl;
		cout << "sent: " << forecast << endl;
		cout << "received: " << received.erase(0, 1) << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}
	
	cout << "testing read_forecast(): ";
	answer = OK;
	forecast = "";
	received = transmit("f");
	sleep(1);
	if(received[0] != '0') {
		cout << "failed" << endl;
		cout << "expected: 0" << endl;
		cout << "received: " << received[0] << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}
	
	// maximal message size
	cout << "testing maximal message size using send_status()" << endl;
	answer = OK;
	sent = "alles klar hier";	// 15 bytes
	received = "";
	int i = 0;
	try {
		connect();
		send("s");
		for(i = 0; i < 500; i ++) {
			send(sent);
		}
		shutdown();
		received = receive();
		close();
	}
	catch (Exception &e) {
		cout << "error: " << e.what() << endl;
	}
	catch(...) {
		cout << "unknown error while sending" << endl;
	}
	cout << "could send "  << (i * 15) << " bytes" << endl;
	if(received == "1") {
		cout << "server responded OK: failed" << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}
	
	
	// read timeout
	cout << "testing continuous read timeout using send_status()" << endl;
	answer = OK;
	sent = "alles klar hier";	// 15 bytes
	received = "";
	try {
		connect();
		send("s");
		sleep(62);
		send(sent);
		shutdown();
		received = receive();
		close();
	}
	catch (Exception &e) {
		cout << "error: " << e.what() << endl;
	}
	catch(...) {
		cout << "unknown error while transmitting" << endl;
	}
	if(received == "1") {
		cout << "server responded OK after 63 seconds: failed" << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}

	// accumulated read timeout
	cout << "testing accumulated read timeout using send_status()" << endl;
	answer = OK;
	sent = "alles klar hier";	// 15 bytes
	received = "";
	try {
		connect();
		send("s");
		for(i = 0; i < 10; i ++) {
			send(sent);
			sleep(10);
		}
		shutdown();
		received = receive();
		close();
	}
	catch (Exception &e) {
		cout << "error: " << e.what() << endl;
	}
	catch(...) {
		cout << "unknown error while transmitting" << endl;
	}
	if(received == "1") {
		cout << "server responded OK after 100 seconds: failed" << endl;
		passed = false;
	}
	else {
		cout << "passed" << endl;
	}
	
	// random behavior
	cout << "random behavior: requesting forecast and then immediately close connection." << endl;
	answer = OK;
	forecast = "hallo welt";
	wait = true;
	connect();
	send("f");
	close();
	sleep(5);
	cout << "what did the server do?" << endl;

	cout << "stopping the server... ";
	server.signal_stop();
	server.join();
	cout << "done" << endl;
	return passed;
}

MessageHandler::RequestStatus MessageServerTester::send_status(const std::string &s)
{
	status = s;
	if(wait) {
		sleep(1);
	}
	return answer;
}

MessageHandler::RequestStatus MessageServerTester::receive_message(std::string &message)
{
	message = status;
	return answer;
}


MessageHandler::RequestStatus MessageServerTester::read_forecast(std::vector<MessageHandler::byte> &forecast)
{
	read = true;
	if(wait) {
		sleep(1);
	}
	forecast = vector<MessageHandler::byte>(forecast.begin(), forecast.end());
	return answer;
}

string MessageServerTester::transmit(const string &s)
{
	connect();
	send(s);
	shutdown();
	string read = receive();
	close();
	return read;
}

void MessageServerTester::connect() {
	struct hostent *host;
	struct sockaddr_in server_addr;
	
	host = gethostbyname("127.0.0.1");
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		throw(Exception("creating socket", errno));
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);
	
	if (::connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
		throw(Exception("connecting to server", errno));
	}
	
}

string MessageServerTester::receive()
{
	char recv_data[1024];
	string received;
	int n;
	do {
		n = recv(sock, recv_data, 1024, 0);
		if(n == 0) {
			return received;
		}
		if(n < 0) {
			throw(Exception("while receiving", errno));
		}
		recv_data[n] = '\0';
		received.append(recv_data);
	} while(1);
}

void MessageServerTester::shutdown()
{
	if(::shutdown(sock, SHUT_WR) != 0) {
		throw(Exception("shuting down outgoing connection", errno));
	}
}

void MessageServerTester::send(const string &data)
{
	int n;
	unsigned int sent = 0;
	do {
		
		n = write(sock, data.c_str() + sent, data.size() - sent);
		if (n < 0) {
			throw(Exception("writing to socket", errno));
		}
		sent += n;
	} while(sent < data.size());		
	
}

void MessageServerTester::close()
{
	::close(sock);
}
