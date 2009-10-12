//
// C++ Implementation: GPSTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GPSTester.h"
#include "GPSSource.h"

#include <unistd.h>

#include <iostream>

using namespace std;

GPSTester::GPSTester()
 : Tester()
{
}


GPSTester::~GPSTester()
{
}


bool GPSTester::test()
{
	GPSSource gps;
	cout << "connecting to GPS module... " << flush;
	gps.connect();
	cout << "done" << endl;
	
	sleep(2);
	cout << "reading position: " << flush;
	GPSSource::Position pos = gps.read_position();
	cout << "latitude: " << pos.latitude << " longitude: " << pos.longitude << endl;
	
	cout << "disconnecting from GPS module... " << flush;
	gps.disconnect();
	cout << "done" << endl;
	return true;
}

