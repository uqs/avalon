//
// C++ Interface: SharedMemoryTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SMTESTER_H
#define SMTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class SharedMemoryTester : public Tester
{
public:
    SharedMemoryTester();

    virtual ~SharedMemoryTester();

    virtual bool test();

};

#endif
