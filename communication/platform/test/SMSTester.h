//
// C++ Interface: SMSTester
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SMSTESTER_H
#define SMSTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class SMSTester : public Tester {
public:
	SMSTester();

	virtual ~SMSTester();
	virtual bool test();
	bool test_encode();
	bool test_decode();

};

#endif
