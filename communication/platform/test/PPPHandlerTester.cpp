//
// C++ Implementation: PPPHandlerTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PPPHandlerTester.h"
#include "IridiumHandler.h"
#include "GPRSHandler.h"
#include "Exception.h"

#include <iostream>

using namespace std;

PPPHandlerTester::PPPHandlerTester()
{
}

PPPHandlerTester::~PPPHandlerTester()
{
}

bool PPPHandlerTester::test_send_sms(PPPHandler &handler)
{
	string message;
	cout << "enter message text: " << flush;
	getline(cin, message);
	cout << "trying to send sms message" << endl;
	handler.send_short_message(message);
	return true;
}

bool PPPHandlerTester::test_receive_sms(PPPHandler &handler)
{
	string message;
	cout << "trying to fetch sms message" << endl;
	cout << handler.poll_short_message() << endl;
	return true;
}

bool PPPHandlerTester::test_ppp(PPPHandler &handler)
{
	bool success = true;
	if(handler.is_available()) {
		cout << "service is available" << endl;
	}
	else {
		cout << "service is not available" << endl;
	}

	if(handler.is_connected()) {
		cout << "already connected" << endl;
	}
	else {
		cout << "not connected" << endl;
	}

	cout << "opening connection..." << flush;
	handler.open_ip_connection();
	cout << "done" << endl;

	if(handler.is_connected()) {
		cout << "connected" << endl;
	}
	else {
		cout << "not connected" << endl;
	}
	char key;
	cout << "press any key to continue" << endl;
	cin >> key;
	cout << "closing connection..." << flush;
	handler.close_ip_connection();
	cout << "done" << endl;

	if(handler.is_connected()) {
		cout << "connected" << endl;
	}
	else {
		cout << "not connected" << endl;
	}
	return success;
}

bool PPPHandlerTester::test_handler(PPPHandler &handler)
{
	bool success = true, s;
	
	cout << "watcha wanna do with this handler?" << endl;
	vector<string> choices;
	choices.push_back("send SMS");
	choices.push_back("receive SMS");
	choices.push_back("open PPP connection");
	choices.push_back("go back");
	
	do {
		unsigned int c = ask(choices);
		s = false;
		try {
			switch(c) {
				case 0:
					s = test_send_sms(handler);
					break;
				case 1:
					s = test_receive_sms(handler);
					break;
				case 2:
					s = test_ppp(handler);
					break;
				case 3:
					return success;
				default:
					continue;
			}
		}
		catch(std::exception& e) {
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

bool PPPHandlerTester::test()
{
	bool success = true, s;
	cout << "which ppp connection should be tested?" << endl;
	vector<string> choices;
	choices.push_back("Iridium");
	choices.push_back("GPRS");
	choices.push_back("go back");
	
	PPPHandler *handler;
	do {
		unsigned int c = ask(choices);
		switch(c) {
			case 0:
				handler = new IridiumHandler(true);
				break;
			case 1:
				handler = new GPRSHandler(true);
				break;
			case 2:
				return success;
			default:
				continue;
		}
		s = false;
		try {
			s = test_handler(*handler);
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



