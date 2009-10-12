//
// C++ Interface: VideoSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include "DataSource.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"

#include <string>



/**
 * Acquires videos from a webcam using ffmpeg.
*/
class VideoSource : public DataSource {
private:
	configlib::configitem<std::string> device_file;
	configlib::configitem<std::string> output_directory;
	configlib::configitem<int> GPIO_port;

	virtual void turn_off();
	virtual void turn_on();
	virtual DataPoint acquire();
	virtual void write_to_stream(std::ostream &os);


public:
	VideoSource(configlib::configfile& config);
	~VideoSource();
};


#endif
