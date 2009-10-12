//
// C++ Implementation: GPRSHandler
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GPRSHandler.h"
#include "GPIO.h"

#include <sys/stat.h>


GPRSHandler::GPRSHandler(bool server) : PPPHandler(50, "gprs", server)
{

	// turn on the GSM modem
	GPIO ignition(64);
	ignition.set_high();
	sleep(1);
	ignition.input();
}


GPRSHandler::~GPRSHandler()
{
}

Serial GPRSHandler::get_serial_port()
{
	std::string device = "/dev/ttyS1";
	Serial ser;
	ser.open(device, B115200, true);
	return ser;
}

std::string GPRSHandler::get_service_center_number()
{
	return "+41794999000";
}


bool GPRSHandler::is_available()
{
	/// @todo
	struct stat state;
	if(stat("/etc/gprs.disable", &state) == 0) {
		return false;
	}
	return true;
}

void GPRSHandler::close_ip_connection() throw()
{
	PPPHandler::close_ip_connection();
}

void GPRSHandler::open_ip_connection()
{
	PPPHandler::open_ip_connection();
}

bool GPRSHandler::send_short_message(std::string message)
{
	return PPPHandler::send_short_message(message);
}
