//
// C++ Implementation: ElectricalSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ElectricalSource.h"
#include "Exception.h"
#include "SoftwareWatchdog.h"

#include <fstream>
#include <sstream>

ElectricalSource::ElectricalSource(configlib::configfile& config) :
		DataSource("Electrical", 0, SoftwareWatchdog::ID_ELECTRICAL, PowerManager::NONE),
		device_file_basename(config, "Electrical", "device file_basename", "", "/proc/ad77x8/ain"),
		shared_data(SharedMemory<Measurement>::ID_ELECTRICAL_DATA, true)
{
	for(int i = 0; i < number_channels; i ++) {
		calibration_data[i].offset = 0;
		calibration_data[i].gain = 1;
	}
	// V_external
	calibration_data[0].offset = -186.;
	calibration_data[0].gain = 1.0128;
	// I_external
	calibration_data[1].offset = -5.8;
	calibration_data[1].gain = .9971;
//	V_battery
//	I_out
	calibration_data[3].offset = -309.;
	calibration_data[3].gain = 0.9162;
	
/*	float V_in;
	float I_in;
	float V_5V;
	float V_node;
	float I_node;
	float V_42V;
*/	
	interpretation_data[0] = 11.;
	interpretation_data[1] = 10.;
	interpretation_data[2] = 8.;
	interpretation_data[3] = 20.;
	interpretation_data[4] = 8.;
	interpretation_data[5] = 2.;
	interpretation_data[6] = 2.96;
	interpretation_data[7] = 2.;
	interpretation_data[8] = 20.;
	interpretation_data[9] = 2.;
}


ElectricalSource::~ElectricalSource()
{
}

DataSource::DataPoint ElectricalSource::acquire()
{
	DataPoint ret;
	ElectricalSource::Measurement m = read();
	ret.data.reserve(number_channels);
	ret.timestamp = m.timestamp;
	ret.data.insert(ret.data.end(), m.data.channels, &m.data.channels[number_channels]);
	return ret;
}


/*!
    \fn ElectricalSource::read()
 */
ElectricalSource::Measurement ElectricalSource::read()
{
	Measurement m;
	float value;
	time(&m.timestamp);
	for(int i = 0; i < number_channels; i++) {
		std::ostringstream filename;
		filename << (std::string)device_file_basename << (i+1);
		std::ifstream in(filename.str().c_str());
		if(!in.is_open()) {
			throw(Exception("couldn't open measurement file"));
		}
		in >> value;
		value *= interpretation_data[i];
		value += calibration_data[i].offset;
		value *= calibration_data[i].gain;
		in.close();
		m.data.channels[i] = value;
	}
	shared_data.write(m);
	return m;
}

void ElectricalSource::write_to_stream(std::ostream &os)
{
	ElectricalSource::Measurement m = read();
	os << "input voltage: " << m.data.names.V_in << " mV" << std::endl;
	os << "current into baseboard: " << m.data.names.I_in << " mA" << std::endl;
	os << "current to control PC: " << m.data.names.I_out << " mA" << std::endl;
	os << "5V voltage: " << m.data.names.V_5V << " mV" << std::endl;
	os << "4.2V voltage: " << m.data.names.V_42V << " mV" << std::endl;
	os << "node voltage: " << m.data.names.V_node << " mV" << std::endl;
	os << "node current: " << m.data.names.I_node << " mA" << std::endl;
	os << "external voltage: " << m.data.names.V_external << " mV" << std::endl;
	os << "external current: " << m.data.names.I_external << " mA" << std::endl;
//	os << "n/c: " << m.names.V_battery << std::endl;
}

