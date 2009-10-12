//
// C++ Implementation: MessageManagerTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MessageManagerTester.h"

#include "MessageManager.h"
#include "EthernetHandler.h"

#include <unistd.h>

#include <iostream>
#include <fstream>

using namespace std;

MessageManagerTester::MessageManagerTester()
{
}


MessageManagerTester::~MessageManagerTester()
{
}

/*!
    \fn MessageManagerTester::test()
 */
bool MessageManagerTester::test()
{
	filebuf logfile;
	logfile.open("mmtest.log", ios::out|ios::trunc);
	if(!logfile.is_open()) {
		throw(Exception("error opening logfile"));
	}
	cout << "server logfile is in ./mmtest.log" << endl;
	cout << "config file read from ./mmtest.conf" << endl;
	configlib::configfile config("mmtest.conf");
	
	bool success = true;
	char request[] = "46.422713, 9.041749";
	MessageManager mm(config);
	config.read();
	config.write();
	EthernetHandler handler;
	mm.register_transport_handler(&handler);
	mm.set_logfile(&logfile);
	
	cout << "starting MessageManager" << endl;
	mm.start();
	cout << "give it some time to fetch mail" << endl;
	sleep(10);
	
	cout << "sending status message... " << flush;
	mm.send_status(request);
	cout << "done" << endl;
	
	sleep(10);
	cout << "stopping MessageManager... ";
	mm.signal_stop();
	mm.join();
	cout << "done" << endl;
	return success;
}
