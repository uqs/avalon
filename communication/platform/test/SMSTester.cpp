//
// C++ Implementation: SMSTester
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SMSTester.h"
#include "SMS.h"

#include <iostream>

using namespace std;

SMSTester::SMSTester()
{
}


SMSTester::~SMSTester()
{
}



bool SMSTester::test_decode()
{
	SMS sms;
	string data;
	cout << "enter message data: " << flush;
	getline(cin, data);
	sms.set_data(data);
	sms.decode();
	cout << "data length: " << sms.get_data_length() << endl;
	cout << "sender number: " << sms.get_origin_number() << endl;
	cout << "decoded message: " << endl;
	cout << sms.get_text() << endl;
	
	return true;
}

bool SMSTester::test_encode()
{
	SMS sms;
	string text;
	cout << "enter message text: " << flush;
	getline(cin, text);
	sms.set_text(text);
	sms.set_destination_number("+41774573725");
	sms.set_service_center_number("+41794999000");
	sms.encode();
	cout << "data length: " << sms.get_data_length() << endl;
	cout << "encoded message: " << endl;
	cout << sms.get_data() << endl;
	
	return true;
}

bool SMSTester::test()
{
	bool success = true, s;
	vector<string> choices;
	choices.push_back("encode");
	choices.push_back("decode");
	choices.push_back("go back");
	
	do {
		unsigned int c = ask(choices);
		try {
			s = false;
			switch(c) {
				case 0:
					s = test_encode();
					break;
				case 1:
					s = test_decode();
					break;
				case 2:
					return success;
				default:
					continue;
			}
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
