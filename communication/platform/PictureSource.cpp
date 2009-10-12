//
// C++ Implementation: PictureSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PictureSource.h"
#include "Exception.h"
#include "GPIO.h"
#include "SoftwareWatchdog.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>


PictureSource::PictureSource(configlib::configfile& config) :
		DataSource("Picture", 10, SoftwareWatchdog::ID_PICTURE, PowerManager::TAKE_PICTURE),
		device_file(config, "Camera", "device file", "", "/dev/video0"),
		output_directory(config, "Picture", "output directory", "", "/media/card/logs/pictures"),
		GPIO_port(config, "Camera", "GPIO port", "", 73)
{
}


PictureSource::~PictureSource()
{
}

void PictureSource::turn_off()
{
	GPIO gpio(GPIO_port);
	gpio.input();
}

void PictureSource::turn_on()
{
	GPIO gpio(GPIO_port);
	gpio.set_high();
}

DataSource::DataPoint PictureSource::acquire()
{
	DataPoint ret;
	std::ostringstream command;
	ret.timestamp = time(NULL);
	command << "uvccapture -m -x960 -y720 -q85";
	command << " -d" << (std::string)device_file;
	command << " -o" << (std::string)output_directory << "/pic-`date +%Y-%m-%d-%H:%M`.jpg";
	if(system(command.str().c_str()) != 0) {
		throw(Exception("starting uvccapture"));
	}
	return ret;
}

void PictureSource::write_to_stream(std::ostream &os)
{
	acquire();
	os << "picture taken";
}


