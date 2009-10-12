//
// C++ Implementation: PowerManager
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PowerManager.h"

const PowerManager::TaskLimits PowerManager::limits[LAST_ACTION] = {
	{  2, 0 },
	{  1, 0 },
	{ 48, 1 },
	{ 24, 6 },
	{  2, 1 },
	{ 24, 0 },
	{  3, 0 },
	{  1, 0 },
	{  1, 0 },
	{  1, 0 },
	{  1, 0 },
	{ 10, 0 }
};


PowerManager::PowerManager(bool server) : global_data(SharedMemory<PowerData>::ID_POWER_DATA, server), is_server(server)
{
	if(server) {
		PowerData data;
		for(int i = 0; i < LAST_ACTION; i++) {
			data.runs[i] = limits[i].min;
		}
		global_data.write(data);
	}
	else {
		PowerData data = global_data.read();
		last_reschedule = data.timestamp;
	}
	for(int i = 0; i < LAST_ACTION; i++) {
		runs_executed[i] = 0;
	}
}


PowerManager::~PowerManager()
{
}




/*!
    \fn PowerManager::is_allowed(PowerManager::Action action)
 */
bool PowerManager::is_allowed(PowerManager::Action action)
{
	if(action == NONE) {
		return true;
	}
	PowerData data = global_data.read();
	if(last_reschedule != data.timestamp) {
		last_reschedule = data.timestamp;
		for(int i = 0; i < LAST_ACTION; i++) {
			runs_executed[i] = 0;
		}
	}
	
	time_t tod = time(NULL) - data.timestamp;
	if((runs_executed[action] / (float)data.runs[action]) <= (tod / (3600.0 * 24))) {
		return true;
	}
	return false;
}


/*!
    \fn PowerManager::inform_execution(PowerManager::Action action)
 */
void PowerManager::inform_execution(PowerManager::Action action)
{
	runs_executed[action]++;
}

/*!
    \fn PowerManager::schedule()
 */
void PowerManager::schedule()
{
	if(!is_server) {
		throw(Exception("wrong role to recalculate schedule"));
	}
	PowerData data;
	// dumbest scheduler: everything is allowed
	for(int i = 0; i < LAST_ACTION; i++) {
		data.runs[i] = limits[i].max;
	}
	global_data.write(data);
	data.timestamp = time(NULL);
	global_data.write(data);
}

