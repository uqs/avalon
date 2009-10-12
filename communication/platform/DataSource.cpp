//
// C++ Implementation: DataSource
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DataSource.h"


/**
 * @param nam	name of data source. is used for logfile names, error massages, etc.
 * @param setuptime	time, data source has to be turned on to guarantee valid data
 */
DataSource::DataSource(std::string source_name, time_t setuptime, key_t wd_id, PowerManager::Action pm_action) : name(source_name), setup_time(setuptime), watchdog_id(wd_id), power_manager_action(pm_action)
{
	turn_on();
}


DataSource::~DataSource()
{
	turn_off();
}


/**
 *
 */
DataSource::DataPoint DataSource::read_current()
{
	return acquire();
}


/**
 *
 */
std::string DataSource::get_name()
{
	return name;
}

time_t DataSource::get_setup_time()
{
	return setup_time;
}

key_t DataSource::get_watchdog_id()
{
	return watchdog_id;
}

PowerManager::Action DataSource::get_pm_action()
{
	return power_manager_action;
}

std::ostream &operator<<(std::ostream &o, DataSource &s)
{
	s.write_to_stream(o);
	return o;
}
