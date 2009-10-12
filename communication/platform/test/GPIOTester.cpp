//
// C++ Implementation: GPIOTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GPIOTester.h"
#include "Exception.h"

#include <unistd.h>

#include <iostream>

using namespace std;

GPIOTester::GPIOTester()
 : Tester(), gpio(NULL)
{
}


GPIOTester::~GPIOTester()
{
}


bool GPIOTester::test()
{
	int number;
	cout << "enter number of GPIO to test: " << flush;
	cin >> number;
	gpio = new GPIO(number);
	bool success = true;
	vector<string> choices;
	choices.push_back("read value");
	choices.push_back("set as input");
	choices.push_back("set as output high");
	choices.push_back("set as output low");
	choices.push_back("go back");
	
	do {
		cout << endl << "using GPIO" << number << ", ";
		unsigned int c = ask(choices);
		try {
			switch(c) {
				case 0:
					test_read();
					break;
				case 1:
					test_input();
					break;
				case 2:
					test_set_high();
					break;
				case 3:
					test_set_low();
					break;
				case 4:
					return success;
				default:
					continue;
			}
		}
		catch(Exception& e) {
			cout << "error: " << e.what() << endl;
			success = false;
		}
		catch(...) {
			cout << "unknown error occurred" << endl;
			success = false;
		}
	} while(1);

	return true;
}



/*!
    \fn GPIOTester::test_set_high()
 */
void GPIOTester::test_set_high()
{
	gpio->set_high();
	cout << "port was set high" << endl;
}


/*!
    \fn GPIOTester::test_input()
 */
void GPIOTester::test_input()
{
	gpio->input();
	cout << "port was set as input" << endl;
}


/*!
    \fn GPIOTester::test_read()
 */
void GPIOTester::test_read()
{
	if(gpio->is_set()) {
		cout << "port is high" << endl;
	}
	else {
		cout << "port is low" << endl;
	}
}


/*!
    \fn GPIOTester::test_set_low()
 */
void GPIOTester::test_set_low()
{
	gpio->set_low();
	cout << "port was set low" << endl;
}
