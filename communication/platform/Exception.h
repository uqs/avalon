//
// C++ Interface: Exception
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class Exception : public std::exception
{
	std::string message;
public:
	
	Exception(std::string m);
	Exception(std::string m, int errno);
	virtual ~Exception() throw();
	virtual const char* what() const throw();

};

class TimeoutException : public Exception
{
public:
	TimeoutException(std::string m) : Exception(m) {};

};

#endif
