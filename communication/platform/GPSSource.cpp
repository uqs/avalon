//
// C++ Implementation: GPSSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GPSSource.h"
#include "Exception.h"
#include "GPIO.h"
#include "SoftwareWatchdog.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>


GPSSource::GPSSource(configlib::configfile& config) :
		DataSource("GPS", 60, SoftwareWatchdog::ID_GPS, PowerManager::READ_GPS),
		device_file(config, "GPS", "device file", "", "/dev/ttyACM0"),
		GPIO_port(config, "GPS", "GPIO port", "", 66),
		shared_data(SharedMemory<Position>::ID_GPS_DATA, true)
{
}


GPSSource::~GPSSource()
{
}

void GPSSource::turn_off()
{
	// make sure it's turned off
	GPIO gpio(GPIO_port);
	gpio.input();
}

void GPSSource::turn_on()
{
	// make sure it's turned on
	GPIO gpio(GPIO_port);
	gpio.set_high();
}

DataSource::DataPoint GPSSource::acquire()
{
	DataPoint ret;
	Position pos = read_position();
	ret.timestamp = pos.timestamp;
	ret.data.reserve(2);
	ret.data.push_back(pos.latitude);
	ret.data.push_back(pos.longitude);
	return ret;
}


/*!
    \fn GPSSource::read_position()
 */
GPSSource::Position GPSSource::read_position()
{
	std::string line, word;
	double value;
	int lines;

	std::ifstream device(((std::string)device_file).c_str());
	if(!device.good()) {
		throw(Exception("couldn't open device file :" + (std::string)device_file));
	}
	std::istringstream wordstream;
	Position pos;
	
	for(lines = 0; lines < 30 && device.good();	lines++) {
		std::getline(device, line);
		std::istringstream linestream(line);
		// get the name of the data set
		std::getline(linestream, word, ',');
		if(word != "$GPGGA") {
			continue;
		}
		// GMT
		linestream >> value;
		linestream.ignore();
		// latitude
		linestream >> std::setw(2) >> word;
		wordstream.str(word); wordstream.clear();
		wordstream >> pos.latitude;
		linestream >> value;
		pos.latitude += value / 60.;
		linestream.ignore();
		// north or south
		std::getline(linestream, word, ',');
		if(word == "S") {
			pos.latitude = - pos.latitude;
		}
		// longitude
		linestream >> std::setw(3) >> word;
		wordstream.str(word); wordstream.clear();
		wordstream >> pos.longitude;
		linestream >> value;
		pos.longitude += value / 60.;
		linestream.ignore();
		// east or west
		std::getline(linestream, word, ',');
		if(word == "W") {
			pos.longitude = - pos.longitude;
		}
		// quality
		linestream >> pos.quality;
		break;
	}
	if(lines == 30) {
		throw(Exception("couldn't find a valid GPGGA stanza"));
	}
	
	time(&pos.timestamp);

	shared_data.write(pos);
	return pos;
}

void GPSSource::write_to_stream(std::ostream &os)
{
	GPSSource::Position p = read_position();
	os << "latitude: " << p.latitude << ", longitude: " << p.longitude;
}


