//
// C++ Interface: DataSourceTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DATASOURCETESTER_H
#define DATASOURCETESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class DataSourceTester : public Tester
{
public:
    DataSourceTester();

    virtual ~DataSourceTester();

    virtual bool test();

};

#endif
