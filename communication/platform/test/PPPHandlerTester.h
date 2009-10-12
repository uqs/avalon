//
// C++ Interface: PPPHandlerTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PPPHANDLERTESTER_H
#define PPPHANDLERTESTER_H

#include "Tester.h"

#include "PPPHandler.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class PPPHandlerTester : public Tester {
	
public:
	PPPHandlerTester();
	virtual ~PPPHandlerTester();
	virtual bool test();
	bool test_send_sms(PPPHandler &handler);
	bool test_receive_sms(PPPHandler &handler);
	bool test_ppp(PPPHandler &handler);
	bool test_handler(PPPHandler &handler);
};

#endif
