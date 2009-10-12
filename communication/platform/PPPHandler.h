//
// C++ Interface: PPPHandler
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PPPHANDLER_H
#define PPPHANDLER_H

#include "TransportHandler.h"
#include "Serial.h"
#include "SMS.h"

#include <semaphore.h>

#include <string>

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class PPPHandler : public TransportHandler
{
private:
	std::string connection_name;
	sem_t *s_open, *s_close;
	bool connected;
	bool server;
	
	static const std::string SEMAPHORE_NAME_PREFIX;
	static const std::string PPP_LAUNCH_COMMAND;
	static const int PPP_SETUP_TIMEOUT;
	
	virtual Serial get_serial_port();
	virtual std::string get_service_center_number();
	void send_sms(SMS message, Serial ser);
	SMS receive_sms(Serial ser);
public:
	PPPHandler(int cost, std::string name, bool is_server);
	
	virtual ~PPPHandler();
	
	virtual void close_ip_connection() throw();
	virtual bool is_available();
	virtual bool is_connected();
	virtual void open_ip_connection();
	virtual std::string get_name() const;
	virtual bool send_short_message(std::string message);
	virtual std::string poll_short_message();

	bool notify_up();
	bool notify_down();
};

#endif
