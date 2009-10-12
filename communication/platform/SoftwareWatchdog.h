//
// C++ Interface: SoftwareWatchdog
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SOFTWAREWATCHDOG_H
#define SOFTWAREWATCHDOG_H

#include <SharedMemory.h>

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class SoftwareWatchdog
{
	// data members
private:
	SharedMemory<unsigned int> value;
	bool watcher;
	time_t timestamp;
	unsigned int last_value;
	time_t timeout;
public:
	enum ids {
		ID_MESSAGEMANAGER = 1,
		ID_MESSAGESERVER = 2,
		ID_GPS = 3,
		ID_ELECTRICAL = 4,
		ID_ENVIRONMENT = 5,
		ID_PICTURE = 6,
		ID_VIDEO = 7
	};
public:
	SoftwareWatchdog(key_t id, bool is_watcher);
	~SoftwareWatchdog();
	
	bool check();
	void serve();
	void set_timeout(time_t to);

};

#endif
