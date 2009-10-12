#include "Logstream.h"
#include "Exception.h"
#include "PowerManager.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"
#include "utils.h"

#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

// constants
const string logfilename = "pm.log";

// global
Logstream info("pm", cout.rdbuf());
Logstream error("pm", cerr.rdbuf());
bool exit_flag = false;


// declarations
extern "C" void signal_handler(int signal);

/** @file
 *
 * PowerManager for everything
 */

int main(int argc, char *argv[])
{
	filebuf logfile;
	
	try {
		configlib::configfile config("/etc/pm.conf");
		configlib::configitem<bool> do_fork(config, "main", "fork", "f", false);
		
		config.read();
		config.parse(argc, argv);
		
		if((bool)do_fork) {
			detach(logfilename, logfile);
			info << "detached sucessfully" << endl;
		}
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGUSR1, signal_handler);

		PowerManager pm(true);
		
		info << "PowerManager started" << endl;

		while(!exit_flag) {
			
			
			// wait a bit
			/// @todo schedule the rescheduling
			sleep(10);
		}
		info << "PowerManager exited" << endl;
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
