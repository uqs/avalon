//
// C++ Interface: GPIOTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GPIOTESTER_H
#define GPIOTESTER_H

#include "Tester.h"
#include "GPIO.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class GPIOTester : public Tester
{
	GPIO *gpio;
public:
	GPIOTester();
	
	virtual ~GPIOTester();
	
	virtual bool test();
	void test_set_high();
	void test_input();
	void test_read();
	void test_set_low();

};

#endif
