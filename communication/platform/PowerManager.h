//
// C++ Interface: PowerManager
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include "SharedMemory.h"

#include <time.h>

/**
	@author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
*/
class PowerManager
{

public:
	// data types
	enum Action {
		NONE = -1,
		TAKE_PICTURE = 0,
		SHOOT_VIDEO = 1,
		READ_GPS = 2,
		FETCH_COMMAND = 3,
		SEND_STATUS = 4,
		POWER_ON_IRIDIUM = 5,
		OPEN_DATA_CONNECTION = 6,
		FETCH_FORECAST = 7,
		SEND_PICTURE = 8,
		SEND_VIDEO = 9,
		SEND_CURRENT_LOG = 10,
		SEND_PAST_LOG = 11,
		LAST_ACTION
	};
	

private:
	struct PowerData {
		int runs[LAST_ACTION];
		time_t timestamp;
	};
	
	struct TaskLimits{
		int min, max;
	};

	SharedMemory<PowerData> global_data;
	bool is_server;
	static const TaskLimits limits[LAST_ACTION];
	int runs_executed[LAST_ACTION];	///< runs completed since the last reschedule
	time_t last_reschedule;

public:
	// methods
	PowerManager(bool server);
	~PowerManager();

	bool is_allowed(PowerManager::Action action);
	void schedule();
	void inform_execution(PowerManager::Action action);

};

#endif
