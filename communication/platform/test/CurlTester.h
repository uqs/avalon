//
// C++ Interface: CurlTester
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CURLTESTER_H
#define CURLTESTER_H

#include "Tester.h"

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class CurlTester : public Tester {
public:
	CurlTester();

	virtual ~CurlTester();
	virtual bool test();
	bool test_fetch();
	bool test_put_string();
	bool test_put_file();
};

#endif
