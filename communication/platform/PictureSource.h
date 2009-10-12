//
// C++ Interface: PictureSource
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PICTURESOURCE_H
#define PICTURESOURCE_H

#include "DataSource.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"

#include <string>



/**
 * Acquires pictures from a webcam using uvccapture.
*/
class PictureSource : public DataSource {
private:
	configlib::configitem<std::string> device_file;
	configlib::configitem<std::string> output_directory;
	configlib::configitem<int> GPIO_port;

	virtual void turn_off();
	virtual void turn_on();
	virtual DataPoint acquire();
	virtual void write_to_stream(std::ostream &os);


public:
	PictureSource(configlib::configfile& config);
	~PictureSource();
};


#endif
