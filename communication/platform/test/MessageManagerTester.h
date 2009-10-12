//
// C++ Interface: MessageManagerTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
//
#ifndef MESSAGEMANAGERTESTER_H
#define MESSAGEMANAGERTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class MessageManagerTester : public Tester {
public:
	MessageManagerTester();
	
	virtual ~MessageManagerTester();
	virtual bool test();

};

#endif
