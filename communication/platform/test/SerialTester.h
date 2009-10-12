//
// C++ Interface: SerialTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SERIALTESTER_H
#define SERIALTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class SerialTester : public Tester {
public:
	SerialTester();
	
	virtual ~SerialTester();
	virtual bool test();

};

#endif
