//
// C++ Implementation: EnvironmentSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "EnvironmentSource.h"
#include "Exception.h"
#include "SoftwareWatchdog.h"

#include <fstream>

EnvironmentSource::EnvironmentSource(configlib::configfile& config) :
		DataSource("Environment", 0, SoftwareWatchdog::ID_ENVIRONMENT, PowerManager::NONE),
		device_file_temperature(config, "Environment", "device file temperature", "", "/proc/sht1x/temperature"),
		device_file_humidity(config, "Environment", "device file humidity", "", "/proc/sht1x/humidity")
		
{
}


EnvironmentSource::~EnvironmentSource()
{
}

EnvironmentSource::Measurement EnvironmentSource::read()
{
	double val;
	Measurement meas;
	std::ifstream tempstr(((std::string)device_file_temperature).c_str());
	if(!tempstr.good()) {
		throw(Exception("couldn't open temperature file"));
	}
	tempstr >> val;
	meas.temperature = (int)(d1 + d2 * val);
	std::ifstream humstr(((std::string)device_file_humidity).c_str());
	if(!humstr.good()) {
		throw(Exception("couldn't open temperature file"));
	}
	humstr >> val;
	meas.humidity = (int)(c1 + c2 * val + c3 * val * val);
	time(&meas.timestamp);
	return meas;
}

DataSource::DataPoint EnvironmentSource::acquire()
{
	DataPoint point;
	Measurement meas = read();
	point.timestamp = meas.timestamp;
	point.data.push_back(meas.temperature);
	point.data.push_back(meas.humidity);
	return point;
}

void EnvironmentSource::write_to_stream(std::ostream &os)
{
	Measurement meas = read();
	os << "temperature: " << meas.temperature << "°C" << std::endl;
	os << "humidity: " << meas.humidity << "%" << std::endl;
}
