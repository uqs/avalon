//
// C++ Interface: ElectricalTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ELECTRICALTESTER_H
#define ELECTRICALTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class ElectricalTester : public Tester
{
public:
    ElectricalTester();

    virtual ~ElectricalTester();

    virtual bool test();

};

#endif
