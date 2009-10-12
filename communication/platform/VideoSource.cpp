//
// C++ Implementation: VideoSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "VideoSource.h"
#include "Exception.h"
#include "GPIO.h"
#include "SoftwareWatchdog.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>


VideoSource::VideoSource(configlib::configfile& config) :
		DataSource("Picture", 10, SoftwareWatchdog::ID_VIDEO, PowerManager::SHOOT_VIDEO),
		device_file(config, "Camera", "device file", "", "/dev/video0"),
		output_directory(config, "Video", "output directory", "", "/media/card/logs/videos"),
		GPIO_port(config, "Camera", "GPIO port", "", 73)
{
}


VideoSource::~VideoSource()
{
}

void VideoSource::turn_off()
{
	GPIO gpio(GPIO_port);
	gpio.input();
}

void VideoSource::turn_on()
{
	GPIO gpio(GPIO_port);
	gpio.set_high();
}

DataSource::DataPoint VideoSource::acquire()
{
	DataPoint ret;
	std::ostringstream command;
	ret.timestamp = time(NULL);
	command << "ffmpeg -t 5 -f video4linux2 -s 640x480";
	command << "-i " << (std::string)device_file;
	command << " " << (std::string)output_directory << "/mov-`date +%Y-%m-%d-%H:%M`.jpg";
	if(system(command.str().c_str()) != 0) {
		throw(Exception("starting uvccapture"));
	}
	return ret;
}

void VideoSource::write_to_stream(std::ostream &os)
{
	acquire();
	os << "video taken";
}


