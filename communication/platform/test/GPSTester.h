//
// C++ Interface: GPSTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GPSTESTER_H
#define GPSTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class GPSTester : public Tester
{
public:
    GPSTester();

    virtual ~GPSTester();

    virtual bool test();

};

#endif
