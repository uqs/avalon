//
// C++ Interface: IridiumHandler
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IRIDIUMHANDLER_H
#define IRIDIUMHANDLER_H

#include "PPPHandler.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class IridiumHandler : public PPPHandler
{
private:
	virtual Serial get_serial_port();
	virtual std::string get_service_center_number();
public:
	IridiumHandler(bool server);

	~IridiumHandler();

};

#endif
