//
// C++ Implementation: GPIO
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GPIO.h"
#include "Exception.h"

#include <sstream>
#include <fstream>

const std::string GPIO::file_basename("/proc/gpio/GPIO");

/**
 * Opens the GPIO port and checks if it is actually configured for the GPIO
 * function. You cannot change the function of a port using this class.
 */
GPIO::GPIO(int number)
{
	std::ostringstream filename;
	int id;
	filename << file_basename << number;
	device_file_name = filename.str();
	std::ifstream device_file(device_file_name.c_str());
	if(!device_file.is_open()) {
		throw(Exception("opening GPIO device file " + device_file_name));
	}
	std::string mode;
	device_file >> id >> mode;
	if(mode != "GPIO") {
		throw(Exception("port is not configured as GPIO"));
	}
	if(number != id) {
		throw(Exception("port gives inconsistent information"));
	}
	device_file.close();
}


GPIO::~GPIO()
{
}

/**
 * Write a string to the device file.
 */
void GPIO::write_to_device_file(const char *s) {
	std::ofstream device_file(device_file_name.c_str());
	if(!device_file.is_open()) {
		throw(Exception("opening GPIO device file " + device_file_name));
	}
	device_file << s << std::endl;
	
	device_file.close();
}

/**
 * Test if the pin is currently set, no matter if the port is configured as in-
 * or output.
 */
bool GPIO::is_set()
{
	std::string mode, direction, state;
	int number;
	std::ifstream device_file(device_file_name.c_str());
	if(!device_file.is_open()) {
		throw(Exception("opening GPIO device file " + device_file_name));
	}
	device_file >> number >> mode >> direction >> state;
	
	device_file.close();
	
	return state == "set";
}


/**
 * Configure the port as input.
 */
void GPIO::input()
{
	write_to_device_file("in");
}


/**
 * Configure the port as output and set it high.
 */
void GPIO::set_high()
{
	write_to_device_file("out set");
}


/**
 * Configure the port as output and set it high.
 */
void GPIO::set_low()
{
	write_to_device_file("out clear");
}
