//
// C++ Interface: DataLogger
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DATALOGGER_H
#define DATALOGGER_H

#include "Thread.h"
#include "Log.h"
#include "DataSource.h"
#include "SoftwareWatchdog.h"

/**
 * An implementation of Thread that takes care of all the details of logging
 * data from a DataSource to a logfile.
 */
class DataLogger : public Thread, public Log
{
private:
	static const std::string LOG_DIRECTORY;
	static const char SEPARATOR;
	
	DataSource *source;
	time_t period;		/// < intervall in between data acquisitions
	time_t start_offset;	/// < seconds after midnight of first run
	bool rotate_flag;
	
	SoftwareWatchdog watchdog;
	static PowerManager power_manager;
public:
	DataLogger(DataSource *s);
	~DataLogger();
	virtual int run();
	void set_period(int p);
	void rotate();
	void set_start_offset(int s);
	long get_next_timeout();

private:
	void rename_logfile();
};

#endif
