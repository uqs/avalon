//
// C++ Implementation: Tester
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Tester.h"
#include "MessageManagerTester.h"
#include "MessageServerTester.h"
#include "PPPHandlerTester.h"
#include "SMSTester.h"
#include "CurlTester.h"
#include "SerialTester.h"
#include "GPIOTester.h"
#include "DataSourceTester.h"
#include "EthernetTester.h"
#include "SharedMemoryTester.h"
#include "Exception.h"

#include <iostream>
#include <sstream>

using namespace std;

Tester::Tester(int c) : choice(c)
{
}

Tester::Tester() : choice(0)
{
}


Tester::~Tester()
{
}


/*!
    \fn Tester::test
 */
bool Tester::test()
{
	bool success = true, s;
	Tester *tester;
	vector<string> choices;
	choices.push_back("MessageManager");
	choices.push_back("MessageServer");
	choices.push_back("PPPHandler");
	choices.push_back("EthernetHandler");
	choices.push_back("SMS");
	choices.push_back("Serial");
	choices.push_back("DataSource");
	choices.push_back("GPIO");
	choices.push_back("SharedMemory");
	choices.push_back("Curl");
	choices.push_back("exit");
	
	do {
		unsigned int c;
		if(choice == -1) {
			c = ask(choices);
		}
		else {
			c = choice;
		}
		switch(c) {
			case 0:
				tester = new MessageManagerTester;
				break;
			case 1:
				tester = new MessageServerTester;
				break;
			case 2:
				tester = new PPPHandlerTester;
				break;
			case 3:
				tester = new EthernetTester;
				break;
			case 4:
				tester = new SMSTester;
				break;
			case 5:
				tester = new SerialTester;
				break;
			case 6:
				tester = new DataSourceTester;
				break;
			case 7:
				tester = new GPIOTester;
				break;
			case 8:
				tester = new SharedMemoryTester;
				break;
			case 9:
				tester = new CurlTester;
				break;
			case 10:
				return success;
				break;
			default:
				continue;
		}
		s = false;
		try {
			s = tester->test();
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
		if(choice != -1) {
			break;
		}
	} while(1);
	
	return success;
}



/*!
    \fn Tester::ask(std::vector<std::string> choices)
 */
unsigned int Tester::ask(std::vector<std::string> choices)
{
	cout << "watcha wanna test?" << endl << endl;
	unsigned int i;
	for(i = 0; i < choices.size(); i++) {
		cout << " " << i << ":\t" << choices[i] << endl;
	}
	cout << "? " << flush;
	
	std::string line;
	std::getline(std::cin, line);

	std::istringstream iss(line);

	iss >> i;

	std::cout << std::endl;
	return i;
}

int main(int argc, char *argv[])
{
	try {
		int i = -1;
		if(argc > 1) {
			istringstream in(argv[1]);
			in >> i;
		}
		Tester tester(i);
		if(tester.test()) {
			cout << "all test passed" << endl;
		}
		else {
			cout << "some tests failed" << endl;
			return -1;
		}
	}
	catch(Exception& e) {
		cout << "error: " << e.what() << endl;
		return -1;
	}
	catch(...) {
		cout << "unknown error occurred" << endl;
		return -1;
	}
	return 0;
}

