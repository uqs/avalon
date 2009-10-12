//
// C++ Implementation: PPPHandler
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PPPHandler.h"
#include "SMS.h"
#include "Exception.h"

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <errno.h>
#include <stdlib.h>

#include <sstream>
#include <fstream>

const std::string PPPHandler::SEMAPHORE_NAME_PREFIX = "MM_PPP_";
const std::string PPPHandler::PPP_LAUNCH_COMMAND = "pppd call ";
const int PPPHandler::PPP_SETUP_TIMEOUT = 60;

PPPHandler::PPPHandler(int cost, std::string name, bool is_server) : TransportHandler(cost), connection_name(name), connected(false), server(is_server)
{
	// create semaphore for IPC communication when setting up PPP connection
	// don't fail if semaphore already exists
	int oflag = 0;
	if(server) {
		oflag = O_CREAT;// | O_EXCL;
	}
	s_open = sem_open((SEMAPHORE_NAME_PREFIX + connection_name + "open").c_str(), oflag, 0644 , 0);
	if(s_open == SEM_FAILED) {
		throw(Exception("creating the semaphore for PPP notifications", errno));
	}
	s_close = sem_open((SEMAPHORE_NAME_PREFIX + connection_name + "close").c_str(), oflag, 0644 , 0);
	if(s_close == SEM_FAILED) {
		throw(Exception("creating the semaphore for PPP notifications", errno));
	}
}


PPPHandler::~PPPHandler()
{
	if(is_connected()) {
		close_ip_connection();
	}

	// destroy semaphore
	sem_close(s_open);
	sem_close(s_close);
	if(server) {
		sem_unlink((SEMAPHORE_NAME_PREFIX + connection_name + "open").c_str());
		sem_unlink((SEMAPHORE_NAME_PREFIX + connection_name + "close").c_str());
	}
}


bool PPPHandler::is_available()
{
	return true;
}

bool PPPHandler::is_connected()
{
	return connected;
}

void PPPHandler::open_ip_connection()
{
	// make sure, semaphore will block
	while(sem_trywait(s_open) == 0);
	
	// trigger ppp daemon
	/// @todo: is this safe? shouldn't we fork / use exec?
	if(system((PPP_LAUNCH_COMMAND + connection_name).c_str()) != 0) {
		// pppd failed to start. that shouldn't happen
		throw(Exception("starting pppd daemon"));
	}
	
	// wait for connection to be established
	struct timespec timeout;
	timeout.tv_sec = time(NULL) + PPP_SETUP_TIMEOUT;
	timeout.tv_nsec = 0;
	if(sem_timedwait(s_open, &timeout) != 0) {
		if(errno == ETIMEDOUT) {
			throw(TimeoutException("waiting for pppd to connect"));
		}
	}
	connected = true;
}

void PPPHandler::close_ip_connection() throw()
{
	///@todo make this more sophisticated
	struct timespec timeout;
	timeout.tv_sec = time(NULL) + PPP_SETUP_TIMEOUT;
	system("killall pppd");
	connected = false;
	if(sem_timedwait(s_close, &timeout) != 0) {
		if(errno == ETIMEDOUT) {
			// too bad
//			return false;
		}
	}
	
//	return true;
}

Serial PPPHandler::get_serial_port()
{
	throw(Exception("PPPHandler can't send SMS"));
}

std::string PPPHandler::get_service_center_number()
{
	throw(Exception("PPPHandler can't send SMS"));
}


bool PPPHandler::send_short_message(std::string message)
{
	if(is_connected()) {
		return TransportHandler::send_short_message(message);
	}
	SMS sms;
	sms.set_text(message);
	sms.set_service_center_number(get_service_center_number());
	
	// try to read phone numbers from file
	std::ifstream numbers("/etc/mm.sms");
	if(!numbers.is_open()) {
		sms.set_destination_number("+41774573725");
		sms.encode();
		send_sms(sms, get_serial_port());
	}
	else {
		while(!numbers.eof()) {
			std::string number;
			numbers >> number;
			if(number.size() == 0) {
				continue;
			}
			sms.set_destination_number(number);
			sms.encode();
			send_sms(sms, get_serial_port());
		}
	}
	return true;
}

/**
 * Polls for a pending short message. If more than one message is pending,
 * removes any one from the pending set and returns it.
 *
 * @return a string with the message part, or an empty string if no message is pending.
 */
std::string PPPHandler::poll_short_message()
{
	SMS sms = receive_sms(get_serial_port());
	return sms.get_text();
}

void PPPHandler::send_sms(SMS sms, Serial ser)
{
	/// @todo turn the fucker on!
	int i;
	std::vector<std::string> answers;
	answers.push_back("OK");
	answers.push_back("ERROR");
	std::vector<std::string> prompt;
	prompt.push_back(">");
	prompt.push_back("ERROR");
	
	ser.write("ATZ\r");
	i = ser.wait_for(answers, NULL);
	if(i != 0) {
		throw(Exception("resetting the modem"));
	}
	
	ser.write("AT+CMGF=0\r");
	i = ser.wait_for(answers, NULL);
	if(i != 0) {
		throw(Exception("setting to PDU mode"));
	}
	
	std::ostringstream os;
	os << "AT+CMGS=" << sms.get_data_length() << "\r";
	ser.write(os.str());
	i = ser.wait_for(prompt, NULL);
	if(i != 0) {
		throw(Exception("waiting for SMS prompt"));
	}
	
	ser.set_read_timeout(30);
	ser.write(sms.get_data() + (char)26);	// CTRL-Z is 26
	i = ser.wait_for(answers, NULL);
	if(i != 0) {
		throw(Exception("sending the sms"));
	}
}

SMS PPPHandler::receive_sms(Serial ser)
{
	int i, index;
	SMS sms;
	std::istringstream in;
	std::ostringstream out;
	std::vector<std::string> answers;
	answers.push_back("OK");
	answers.push_back("ERROR");
	
	ser.write("ATZ\r");
	i = ser.wait_for(answers, NULL);
	if(i != 0) {
		throw(Exception("resetting the modem"));
	}
	
	ser.write("AT+CMGF=0\r");
	i = ser.wait_for(answers, NULL);
	if(i != 0) {
		throw(Exception("setting to PDU mode"));
	}
	
	// list ALL messages
	std::string messages;
	ser.write("AT+CMGL=4\r");
	i = ser.wait_for(answers, &messages);
	if(i != 0) {
		throw(Exception("listing messages"));
	}
	
	size_t pos = messages.find("+CMGL:");
	if(pos == std::string::npos) {
		// no message found, return empty sms object
		return sms;
	}
	
	// here find out message nr
	size_t begin = pos + 6;
	pos = messages.find(",", pos);
	// the message index is in messages[begin : pos]
	in.str(messages.substr(begin, pos - begin));
	in >> index;
	
	pos = messages.find("\n", pos);
	if(pos == std::string::npos) {
		throw(Exception("Error parsing message"));
	}
	sms.set_data(messages.substr(pos+1));
	
	//now we delete the message we just read
	out << "AT+CMGD=" << index << "\r";
	ser.write(out.str());
	ser.wait_for(answers, NULL); // ignore errors on this one
	
	sms.decode();
	return sms;
}


bool PPPHandler::notify_up()
{
	if(sem_post(s_open) == 0) {
		return true;
	}
	return false;
}

bool PPPHandler::notify_down()
{
	if(sem_post(s_close) == 0) {
		return true;
	}
	return false;
}

std::string PPPHandler::get_name() const
{
	return "PPPHandler " + connection_name;
}

