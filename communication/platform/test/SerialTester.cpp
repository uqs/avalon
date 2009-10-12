//
// C++ Implementation: SerialTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SerialTester.h"

#include "Serial.h"

#include <iostream>

using namespace std;

SerialTester::SerialTester()
{
}


SerialTester::~SerialTester()
{
}




/*!
    \fn SerialTester::test()
 */
bool SerialTester::test()
{
	Serial ser;
	string input, output, device = "/dev/ttyUSB0";
	ser.open(device, B19200, false);
	
	cout << "opened " << device << ". waiting for your input" << endl;
	
	vector<string> answers;
	answers.push_back("OK");
	answers.push_back("ERROR");
//	ser.write("atz");
//	output = ser.read();
//	cout << output << endl;
	
	do {
		cout << "prompt> ";
		cin >> input;
		ser.write(input + "\r");
		int i = ser.wait_for(answers, NULL);
		cout << "awaited " << answers[i] << endl;
	} while(1);
	
	ser.close();
	return true;
}
