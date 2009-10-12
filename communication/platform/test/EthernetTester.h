//
// C++ Interface: EthernetTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EthernetTESTER_H
#define EthernetTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class EthernetTester : public Tester
{
public:
    EthernetTester();

    virtual ~EthernetTester();

    virtual bool test();

};

#endif
