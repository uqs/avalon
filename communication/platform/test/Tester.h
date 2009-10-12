//
// C++ Interface: Tester
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TESTER_H
#define TESTER_H

#include <vector>
#include <string>

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class Tester
{
	int choice;
public:
	Tester(int c);
	Tester();

	virtual ~Tester();
	unsigned int ask(std::vector<std::string> choices);
	virtual bool test();

};

#endif
