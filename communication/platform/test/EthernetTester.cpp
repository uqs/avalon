//
// C++ Implementation: EthernetTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "EthernetTester.h"
#include "EthernetHandler.h"

#include <iostream>

using namespace std;

EthernetTester::EthernetTester()
 : Tester()
{
}


EthernetTester::~EthernetTester()
{
}


bool EthernetTester::test()
{
	EthernetHandler handler;
	string s;
	cout << "enter short message: " << flush;
	getline(cin, s);
	return handler.send_short_message(s);
}

