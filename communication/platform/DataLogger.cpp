//
// C++ Implementation: DataLogger
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DataLogger.h"

#include <fstream>
#include <sstream>
#include <iomanip>

#include <signal.h>
#include <stdio.h>

const std::string DataLogger::LOG_DIRECTORY("/media/card/logs/");
const char DataLogger::SEPARATOR(';');

PowerManager DataLogger::power_manager(false);

DataLogger::DataLogger (DataSource *s) : Log("DataLogger"), source(s), period(60), start_offset(0), watchdog(s->get_watchdog_id(), false)
{
}


DataLogger::~DataLogger()
{
}

/**
 * Main metod of the DataLogger Thread. Schedules the data acquisition according
 * to the specified period. If the period is larger than the setup time of the
 * data source, the source is switched off in between measurements.
 * Also serves the WatchDog.
 */
int DataLogger::run()
{
	sigset_t listening_signals;
	sigemptyset(&listening_signals);
	sigaddset(&listening_signals, SIGUSR1);
	struct timespec timeout;
	// witch data source on and off if there is enough time
	bool switch_power = period > source->get_setup_time();
	if(!switch_power) {
		source->turn_on();
	}

	// open logfile
	std::ofstream logfile((LOG_DIRECTORY + source->get_name()).c_str(), std::ios::out|std::ios::app);
	if(!logfile.is_open()) {
		return -1;
	}
	
	info << "data logger for " << source->get_name() << " going to main loop" << std::endl;
	while(!done) {
		// calculate the next timeout using startup time and number of runs to avoid drift
		timeout.tv_sec = get_next_timeout();
		watchdog.serve();
		sigtimedwait(&listening_signals, NULL, &timeout);
		watchdog.serve();
		
		if(done) {
			break;
		}
		
		if(rotate_flag) {
			logfile.close();
			rename_logfile();
			logfile.open((LOG_DIRECTORY + source->get_name()).c_str(), std::ios::out|std::ios::app);
			if(!logfile.is_open()) {
				return -1;
			}
			
			rotate_flag = false;
			continue;
		}

		// ask PowerManager if we may
		if(!power_manager.is_allowed(source->get_pm_action())) {
			continue;
		}
		// turn it on and wait the setup time
		if(switch_power) {
			source->turn_on();
			timeout.tv_sec = source->get_setup_time();
			sigtimedwait(&listening_signals, NULL, &timeout);
		}
		// read data
		DataSource::DataPoint point = source->read_current();
		// turn off
		if(switch_power) {
			source->turn_off();
		}
		// write to file
		logfile << point.timestamp;
		for(std::vector<double>::iterator i = point.data.begin(); i != point.data.end(); i++) {
			logfile << SEPARATOR << *i;
		}
		logfile << std::endl;
		// tell PowerManager what we did
		power_manager.inform_execution(source->get_pm_action());
	}
	
	if(!switch_power) {
		source->turn_off();
	}
	info << "data logger " << source->get_name() << " exitting" << std::endl;
	
	logfile.close();
	return 0;
}



/**
 * Period is assumed to divide 24 hours
 */
void DataLogger::set_period(int p)
{
	period = p;
}


/**
 * Signals the Logger to close its logfile, rename it and open a new one.
 */
void DataLogger::rotate()
{
	rotate_flag = true;
	interrupt();
}


/**
 * Renames the current logfile by appending the current date in the form YYYY-MM-DD.
 * If the new file already exists, appends a unique, ascending number.
 */
void DataLogger::rename_logfile()
{
	time_t at;
	struct tm rt;
	struct stat st;
	int i = 0;
	std::ostringstream newname, basename;
	std::string oldname = LOG_DIRECTORY + source->get_name();
	time(&at);
	gmtime_r(&at, &rt);
	basename << std::setfill('0');
	basename << LOG_DIRECTORY << source->get_name() << '-'
			<< rt.tm_year+1900 << '-'
			<< std::setw(2) << rt.tm_mon+1 << '-'
			<< std::setw(2) << rt.tm_mday;
	newname.str(basename.str());
	while(stat(newname.str().c_str(), &st) == 0) {
		// choose a new name if the file exists
		newname.str(basename.str());
		newname << '.' << i++;
	}
	if(rename(oldname.c_str(), newname.str().c_str()) != 0) {
		throw(Exception("couldn't rename log file " + oldname, errno));
	}
}


/*!
    \fn DataLogger::set_start(int s)
 */
void DataLogger::set_start_offset(int s)
{
	start_offset = s;
}


/**
 * Calculates the duration of the next timeout based on the start offset and the
 * period. The timeout will be smaller than the period and expire at
 * start_offset + n * period;
 */
long DataLogger::get_next_timeout()
{
	time_t now;
	struct tm tod;
	long after_midnight;

	time(&now);
	gmtime_r(&now, &tod);
	after_midnight = tod.tm_sec + 60 * tod.tm_min + 3600 * tod.tm_hour;

	if(after_midnight < start_offset) {
		after_midnight += 3600;
	}
	return period - (after_midnight - start_offset) % period;
}
