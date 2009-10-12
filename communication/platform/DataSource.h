//
// C++ Interface: DataSource
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DATASOURCE_H
#define DATASOURCE_H

#include "PowerManager.h"

#include <time.h>

#include <string>
#include <vector>

/**
 * Common interface to all data sources on the communication platform.
 */
class DataSource
{
private:
	std::string name;
	time_t setup_time;
	key_t watchdog_id;
	PowerManager::Action power_manager_action;

public:
	/**
	 * Holds the data of one measurement including a timestamp.
	 */
	struct DataPoint {
		time_t timestamp;
		std::vector<double> data;
	};
	
	DataSource(std::string source_name, time_t setuptime, key_t wd_id, PowerManager::Action pm_action);

	virtual ~DataSource();
	DataPoint read_current();
	std::string get_name();
	time_t get_setup_time();
	key_t get_watchdog_id();
	PowerManager::Action get_pm_action();
	virtual void turn_off() {};
	virtual void turn_on() {};

	friend std::ostream &operator<<(std::ostream &o, DataSource &s);

private:
	virtual DataPoint acquire() = 0;
	virtual void write_to_stream(std::ostream &os) = 0;
};



#endif
