#include "DataLogger.h"
#include "ElectricalSource.h"
#include "EnvironmentSource.h"
#include "PictureSource.h"
#include "VideoSource.h"
#include "Exception.h"
#include "GPSSource.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"
#include "utils.h"

#include <signal.h>
#include <sys/time.h>

#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

// constants
const string logfilename = "dl.log";

// global
Logstream info("dl", cout.rdbuf());
Logstream error("dl", cerr.rdbuf());
bool exit_flag = false;
std::vector<DataLogger *> threads;


// declarations
extern "C" void signal_handler(int signal);

int main(int argc, char *argv[])
{
	filebuf logfile;
	
	try {
		std::vector<DataLogger *>::iterator i;
		
		configlib::configfile config("/etc/dl.conf");
		configlib::configitem<bool> do_fork(    config, "main", "fork",        "f", false);
		configlib::configitem<bool> use_el(     config, "main", "Electrical",  "e", false);
		configlib::configitem<bool> use_gps(    config, "main", "GPS",         "g", true);
		configlib::configitem<bool> use_env(    config, "main", "EnvironmentSource", "t", false);
		configlib::configitem<bool> use_picture(config, "main", "Picture",     "p", false);
		configlib::configitem<bool> use_video(  config, "main", "video",       "v", false);
		
		// construct all logging threads
		info << "constructing threads: ";
		if(use_el) {
			ElectricalSource *el = new ElectricalSource(config);
			DataLogger *ElectricalLogger = new DataLogger(el);
			ElectricalLogger->set_period(30);
			threads.push_back(ElectricalLogger);
			info << "Electrical ";
		}

		if(use_gps) {
			GPSSource *gps = new GPSSource(config);
			DataLogger *GPSLogger = new DataLogger(gps);
			GPSLogger->set_period(30);
			threads.push_back(GPSLogger);
			info << "GPS ";
		}

		if(use_env) {
			EnvironmentSource *env = new EnvironmentSource(config);
			DataLogger *EnvLogger = new DataLogger(env);
			EnvLogger->set_period(300);
			threads.push_back(EnvLogger);
			info << "Environment ";
		}
		
		if(use_picture) {
			PictureSource *pic = new PictureSource(config);
			DataLogger *PicLogger = new DataLogger(pic);
			PicLogger->set_period(6*3600);
			PicLogger->set_start_offset(9 * 3600);
			threads.push_back(PicLogger);
			info << "Picture ";
		}
		
		if(use_video) {
			VideoSource *vid = new VideoSource(config);
			DataLogger *VidLogger = new DataLogger(vid);
			VidLogger->set_period(24*3600);
			VidLogger->set_start_offset(12 * 3600);
			threads.push_back(VidLogger);
			info << "Video ";
		}
		info << std::endl;
		
		if((bool)do_fork) {
			detach(logfilename, logfile);
			info << "detached sucessfully" << endl;
			for(i = threads.begin(); i != threads.end(); i++) {
				(*i)->set_logfile(info.rdbuf());
			}
		}
		
		// set the timer for logfile rotation at midnight
		time_t now;
		struct tm tod;
		struct itimerval timer;
		long after_midnight;
		
		time(&now);
		gmtime_r(&now, &tod);
		after_midnight = tod.tm_sec + 60 * tod.tm_min + 3600 * tod.tm_hour;
		timer.it_value.tv_sec = 24 * 3600 - after_midnight;
		timer.it_value.tv_usec = 0;
		timer.it_interval.tv_sec = 24 * 3600;
		timer.it_interval.tv_usec = 0;
		if(setitimer(ITIMER_REAL, &timer, NULL) != 0) {
			throw(Exception("while setting timer for log rotation", errno));
		}

		// set appropriate signal handlers
		signal(SIGALRM, signal_handler);
		signal(SIGHUP,  signal_handler);
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGUSR1, signal_handler);
		
		info << "starting threads" << endl;
		for(i = threads.begin(); i != threads.end(); i++) {
			(*i)->start();
		}
		
		info << "waiting for threads to exit" << endl;
		for(i = threads.begin(); i != threads.end(); i++) {
			(*i)->join();
		}
	}
	catch(Exception &e) {
		error << "Error " << e.what() << endl;
		return -1;
	}
	catch(...) {
		error << "unknown error" << endl;
		return -1;
	}

	info << "distroying threads" << endl;
	while(threads.size() > 0) {
		DataLogger *l = threads.back();
		threads.pop_back();
		delete l;
	}
	
	return 0;
}


/**
 * Signal handler for termination signals.
 * Flags the signal to the server main loop on the first call and exits on the second.
 */
extern "C" void signal_handler(int signal)
{
	switch(signal) {
		case SIGHUP:
		case SIGALRM:
			for(std::vector<DataLogger *>::iterator i = threads.begin(); i != threads.end(); i++) {
				(*i)->rotate();
			}			
			break;
		case SIGUSR1:
			return;
		default:
			/// @todo we probably shouldn't do I/O here.
			if(exit_flag) {
				error << "Caught second signal, dieing badly" << endl;
				exit(-1);	
			}
		//	info << "Received signal, waiting for servers to quit." << endl;
			exit_flag = true;
			try {
				for(std::vector<DataLogger *>::iterator i = threads.begin(); i != threads.end(); i++) {
					(*i)->signal_stop();
				}
			}
			catch(...) {
			}
	}
}
