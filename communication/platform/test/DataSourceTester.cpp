//
// C++ Implementation: DataSourceTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DataSourceTester.h"
#include "GPSSource.h"
#include "ElectricalSource.h"
#include "EnvironmentSource.h"
#include "PictureSource.h"
#include "VideoSource.h"

#include "Exception.h"

#include <iostream>

using namespace std;

DataSourceTester::DataSourceTester()
 : Tester()
{
}


DataSourceTester::~DataSourceTester()
{
}


bool DataSourceTester::test()
{
	bool success = true, s;
	cout << "which data source should be tested?" << endl;
	vector<string> choices;
	choices.push_back("GPS");
	choices.push_back("ElectricalSource");
	choices.push_back("Environment");
	choices.push_back("Picture");
	choices.push_back("Video");
	choices.push_back("go back");
	
	configlib::configfile config("dl.conf");
	
	DataSource *source;
	do {
		unsigned int c = ask(choices);
		switch(c) {
			case 0:
				source = new GPSSource(config);
				break;
			case 1:
				source = new ElectricalSource(config);
				break;
			case 2:
				source = new EnvironmentSource(config);
				break;
			case 3:
				source = new PictureSource(config);
				break;
			case 4:
				source = new VideoSource(config);
				break;
			case 5:
				return success;
			default:
				continue;
		}
		s = false;
		try {
			cout << (*source);
			
			s = true;
		}
		catch(Exception& e) {
			cout << "error: " << e.what() << endl;
		}
		catch(...) {
			cout << "unknown error occurred" << endl;
		}
		if( s ) {
			cout << "all test passed" << endl;
		}
		else {
			cout << "some tests failed" << endl;
		}
		success &= s;
	} while(1);

	return success;
}

