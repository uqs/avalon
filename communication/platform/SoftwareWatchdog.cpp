//
// C++ Implementation: SoftwareWatchdog
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SoftwareWatchdog.h"


SoftwareWatchdog::SoftwareWatchdog(key_t id, bool is_watcher) : value(id+999, !is_watcher), watcher(is_watcher), timeout(-1)
{
	// initialize wd to served state
	if(watcher) {
		last_value = value.read();
		time(&timestamp);
	}
	else {
		last_value = 1;
		value.write(last_value);
	}
}


SoftwareWatchdog::~SoftwareWatchdog()
{
}




/*!
    \fn SoftwareWatchdog::check()
 */
bool SoftwareWatchdog::check()
{
	if(!watcher) {
		throw(Exception("wrong role for checking watchdog"));
	}
	unsigned int current_value = value.read();
	// either the value changed
	if(current_value != last_value) {
		last_value = current_value;
		time(&timestamp);
		return true;
	}
	else {
		// or the timeout isn't reached yet
		if(timestamp + timeout >= time(NULL)) {
			return true;
		}
	}
	
	return false;
}


/*!
    \fn SoftwareWatchdog::serve()
 */
void SoftwareWatchdog::serve()
{
	if(watcher) {
		throw(Exception("wrong role for serving watchdog"));
	}
	value.write(++last_value);
}


/*!
    \fn SoftwareWatchdog::set_timeout(time_t to)
 */
void SoftwareWatchdog::set_timeout(time_t to)
{
	timeout = to;
}
