//
// C++ Interface: EnvironmentSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "DataSource.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"

#include <time.h>

/**
 * Reads out temperature and humidity measurements from the SHT11 sensor.
 */
class EnvironmentSource : public DataSource
{
// data types
public:
	struct Measurement {
		time_t timestamp;
		int temperature;
		int humidity;
	};
// data members
private:
	configlib::configitem<std::string> device_file_temperature;
	configlib::configitem<std::string> device_file_humidity;

	// constants for sensor value conversions. interpolated for VDD of 3.3V
	static const double c1 =  -4.0;
	static const double c2 =   0.0405;
	static const double c3 =  -2.8e-6;
	static const double d1 = -39.63;
	static const double d2 =   0.01;
	
	
// methods
public:
	EnvironmentSource(configlib::configfile& config);
	~EnvironmentSource();
	Measurement read();

private:
	virtual DataPoint acquire();
	virtual void write_to_stream(std::ostream &os);

};

#endif
