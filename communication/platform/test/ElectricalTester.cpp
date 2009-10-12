//
// C++ Implementation: ElectricalTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ElectricalTester.h"
#include "ElectricalSource.h"

#include <iostream>

using namespace std;

ElectricalTester::ElectricalTester()
 : Tester()
{
}


ElectricalTester::~ElectricalTester()
{
}


bool ElectricalTester::test()
{
	ElectricalSource e;
	ElectricalSource::Measurement m = e.read();
	cout << m;
	return true;
}

