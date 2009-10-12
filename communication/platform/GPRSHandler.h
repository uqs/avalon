//
// C++ Interface: GPRSHandler
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GPRSHANDLER_H
#define GPRSHANDLER_H

#include "PPPHandler.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class GPRSHandler : public PPPHandler
{
private:
	virtual Serial get_serial_port();
	virtual std::string get_service_center_number();
		
public:
	GPRSHandler(bool server);
	~GPRSHandler();

	virtual bool is_available();
	virtual void close_ip_connection() throw();
	virtual void open_ip_connection();
	virtual bool send_short_message(std::string message);

};

#endif
