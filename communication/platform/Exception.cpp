//
// C++ Implementation: Exception
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Exception.h"

#include <string.h>

Exception::Exception(std::string m)
	: std::exception(), message(m)
{
}

Exception::Exception(std::string m, int errno)
	: std::exception(), message(m + ": " + strerror(errno))
{
}

Exception::~Exception() throw()
{
}

const char* Exception::what() const throw()
{
	return message.c_str();
}



