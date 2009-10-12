//
// C++ Interface: GPIO
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GPIO_H
#define GPIO_H

#include <string>

/**
 * Encapsulates one GPIO port on the gumstix (PXA270)
 */
class GPIO {
// data members
private:
	std::string device_file_name;
	static const std::string file_basename;

// methods
private:
	void write_to_device_file(const char *s);
public:
	GPIO(int number);
	~GPIO();

	bool is_set();
	void input();
	void set_high();
	void set_low();
};

#endif
