//
// C++ Interface: GPSSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GPS_H
#define GPS_H

#include "DataSource.h"
#include "SharedMemory.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"

#include <string>



/**
 * Reads position data from a NMEA compatible device.
*/
class GPSSource : public DataSource {
// data types
public:
	typedef struct {
		time_t timestamp;
		double latitude, longitude;
		int quality;
	} Position;

// variables
private:
	configlib::configitem<std::string> device_file;
	configlib::configitem<int> GPIO_port;
	SharedMemory<Position> shared_data;

// methods
private:
	virtual void turn_off();
	virtual void turn_on();
	virtual DataPoint acquire();
	virtual void write_to_stream(std::ostream &os);

public:
	GPSSource(configlib::configfile& config);
	~GPSSource();
	Position read_position();

};


#endif
