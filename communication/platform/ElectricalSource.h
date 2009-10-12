//
// C++ Interface: ElectricalSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ELECTRICAL_H
#define ELECTRICAL_H

#include "DataSource.h"
#include "SharedMemory.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"

#include <string>

/**
 * Reads out and formats data from the A/D converter on the BaseBoard.
*/
class ElectricalSource : public DataSource {
private:
	static const int number_channels = 10;
// types
public:
	typedef struct {
		time_t timestamp;
		union {
			float channels[number_channels];
			struct {
				float V_external;
				float I_external;
				float V_battery;
				float I_out;
				float V_in;
				float I_in;
				float V_5V;
				float V_node;
				float I_node;
				float V_42V;
			} names;
		} data;
	} Measurement;


// variables
private:
	configlib::configitem<std::string> device_file_basename;
	SharedMemory<Measurement> shared_data;

	struct {
		float gain, offset;
	} calibration_data[number_channels];
	float interpretation_data[number_channels];

// methods
public:
	ElectricalSource(configlib::configfile& config);
	~ElectricalSource();
	ElectricalSource::Measurement read();
private:
	virtual DataPoint acquire();
	virtual void write_to_stream(std::ostream &os);

};

#endif
