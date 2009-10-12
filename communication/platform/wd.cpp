#include "GPIO.h"
#include "Logstream.h"
#include "Exception.h"
#include "SoftwareWatchdog.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"
#include "utils.h"

#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// constants
const string logfilename = "wd.log";

// global
Logstream info("wd", cout.rdbuf());
Logstream error("wd", cerr.rdbuf());
bool exit_flag = false;


// declarations
extern "C" void signal_handler(int signal);

/** @file
 *
 * WatchDog for everything
 *  - checks if all (working) threads of all demons are blocked with SoftwareWatchdog (somewhat check)
 *  - kills and restarts demons that are stuck
 *  - serves the hardware watchdog of the gumstix (check)
 *  - checks all (as many as possible) processes on the control computer for deadlocks
 *  - watches free disk space
 *  - blinks the status lamp (check)
 */

int main(int argc, char *argv[])
{
	filebuf logfile;
	
	try {
		configlib::configfile config("/etc/wd.conf");
		configlib::configitem<bool> do_fork(config, "main", "fork", "f", false);
		configlib::configitem<std::string> watchdog_device(config, "main", "watchdog_device", "-w=", "/dev/watchdog");
		
		config.read();
		config.parse(argc, argv);
		
		if((bool)do_fork) {
			detach(logfilename, logfile);
			info << "detached sucessfully" << endl;
		}
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGUSR1, signal_handler);
		
		GPIO led(67);
		std::ofstream watchdog_file(((std::string)watchdog_device).c_str());
		if(!watchdog_file.is_open()) {
			throw(Exception("couldn't open watchdog file"));
		}
		
		SoftwareWatchdog mmdog(SoftwareWatchdog::ID_MESSAGEMANAGER, true);
		mmdog.set_timeout(5*60);
		SoftwareWatchdog msdog(SoftwareWatchdog::ID_MESSAGESERVER, true);
		msdog.set_timeout(2*60);
		SoftwareWatchdog gpsdog(SoftwareWatchdog::ID_GPS, true);
		gpsdog.set_timeout(10*60);
		SoftwareWatchdog electricaldog(SoftwareWatchdog::ID_ELECTRICAL, true);
		electricaldog.set_timeout(10*60);

		info << "WatchDog started" << endl;
		while(!exit_flag) {
			// blink the lamp
			led.set_high();
			sleep(1);
			led.input();
			
			// serve the hardware watchdog
			watchdog_file << "hallo" << std::flush;
			
			// check other threads
			if(!mmdog.check()) {
				error << "MessageManager not responding" << endl;
			}
			if(!msdog.check()) {
				error << "MessageServer not responding" << endl;
			}
			if(!electricaldog.check()) {
				error << "ElectricalSource not responding" << endl;
			}
			if(!gpsdog.check()) {
				error << "GPS not responding" << endl;
			}
			
			// wait a bit
			sleep(10);
		}
		info << "WatchDog terminates" << endl;
	}
	catch(std::exception &e) {
		error << "Error " << e.what() << endl;
		return -1;
	}
	catch(...) {
		error << "unknown error" << endl;
		return -1;
	}
	return 0;
}

/*
 * Some code shamelessly copied from different online sources, such as
 * http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 * http://www.lowtek.com/sockets/select.html
 */

/**
 * Signal handler for termination signals.
 * Flags the signal to the server main loop on the first call and exits on the second.
 */
extern "C" void signal_handler(int signal)
{
	if(signal == SIGUSR1) {
		return;
	}
	/// @todo we probably shouldn't do I/O here.
	if(exit_flag) {
		error << "Caught second signal, dieing badly" << endl;
		exit(-1);	
	}
//	info << "Received signal, waiting for servers to quit." << endl;
	exit_flag = true;
}
