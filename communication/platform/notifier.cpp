//
// C++ Implementation: notifier
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "PPPHandler.h"
#include "Exception.h"

#include <iostream>
#include <string>

using namespace std;

void usage(char *name) {
	cerr << "Usage: " << name << " <connection name> up|down" << endl;
}

int main(int argc, char *argv[])
{
	bool result = false;
	cout << "MessageManager Notificator" << endl;
	if(argc < 2) {
		usage(argv[0]);
		return -1;
	}
	try {
		PPPHandler handler(50, argv[1], false);
		if(argv[2] ==  std::string("up")) {
			result = handler.notify_up();
		}
		else if(argv[2] == std::string("down")) {
			result = handler.notify_down();
		}
		else {
			usage(argv[0]);
			return -1;
		}
	}
	catch(Exception& e) {
		cerr << "Error: " << e.what() << endl;
	}
	
	if(result) {
		cout << "successfully notified MessageManager" << endl;
	}
	else {
		cout << "couldn't notify MessageManager" << endl;
		return -1;
	}
	
	return 0;
}
