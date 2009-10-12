//
// C++ Implementation: IridiumHandler
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "IridiumHandler.h"


IridiumHandler::IridiumHandler(bool server) : PPPHandler(100, "iridium", server)
{
}


IridiumHandler::~IridiumHandler()
{
}

Serial IridiumHandler::get_serial_port()
{
	std::string device = "/dev/ttyS2";
	Serial ser;
	ser.open(device, B19200, false);
	return ser;
}

std::string IridiumHandler::get_service_center_number()
{
	return "+881662900005";
}


