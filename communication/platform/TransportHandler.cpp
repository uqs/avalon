//
// C++ Implementation: TransportHandler
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "TransportHandler.h"
#include "Curl.h"

#include <string>

TransportHandler::TransportHandler(int c) : cost(c)
{
}

TransportHandler::~TransportHandler()
{
}

bool TransportHandler::send_short_message(std::string message) {
	if(is_connected()) {
		// send HTTP GET request
		
		
		Curl curl;
		std::string encoded = curl.urlencode(message);
		curl.fetch("http://pc-10082.ethz.ch:2009/log.php?msg=" + encoded);
		
		return true;
	}
	return false;
}
