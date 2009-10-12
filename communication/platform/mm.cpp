#include "MessageServer.h"
#include "MessageManager.h"
#include "EthernetHandler.h"
#include "IridiumHandler.h"
#include "GPRSHandler.h"
#include "Logstream.h"
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

using namespace std;

// constants
const string logfilename = "mm.log";

// global
Logstream info("mm", cout.rdbuf());
Logstream error("mm", cerr.rdbuf());
bool exit_flag = false;
MessageServer *server = NULL;
MessageManager *manager = NULL;


// declarations
extern "C" void signal_handler(int signal);

int main(int argc, char *argv[])
{
	EthernetHandler* ethernet = NULL;
	IridiumHandler* iridium = NULL;
	GPRSHandler* gprs = NULL;
	filebuf logfile;
	
	try {
		configlib::configfile config("/etc/mm.conf");
		configlib::configitem<bool> use_ethernet(config, "main", "use ethernet", "e", true);
		configlib::configitem<bool> use_gprs(    config, "main", "use gprs",     "g", false);
		configlib::configitem<bool> use_iridium( config, "main", "use iridium",  "i", false);
		configlib::configitem<bool> do_fork(     config, "main", "fork",         "f", false);
		manager = new MessageManager(config);
		server = new MessageServer(config);
		server->set_handler(manager);
		
		config.read();
		config.parse(argc, argv);
		
		if((bool)use_ethernet) {
			info << "using ethernet handler" << endl;
			ethernet = new EthernetHandler;
			manager->register_transport_handler(ethernet);
		}
		if((bool)use_iridium) {
			info << "using iridium handler" << endl;
			iridium = new IridiumHandler(true);
			manager->register_transport_handler(iridium);
		}
		if((bool)use_gprs) {
			info << "using gprs handler" << endl;
			gprs = new GPRSHandler(true);
			manager->register_transport_handler(gprs);
		}
		if((bool)do_fork) {
			detach(logfilename, logfile);
			info << "detached sucessfully" << endl;
			server->set_logfile(info.rdbuf());
			manager->set_logfile(info.rdbuf());
		}
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGUSR1, signal_handler);
		
		server->start();
		manager->start();
		
		// wait for server to exit
		// sigsuspend?
		server->join();
		info << "joined server thread" << endl;
		manager->join();
		info << "joined manager thread" << endl;

	}
	catch(std::exception &e) {
		error << "Error " << e.what() << endl;
		return -1;
	}
	catch(...) {
		error << "unknown error" << endl;
		return -1;
	}
	if(manager != NULL) {
		delete manager;
	}
	if(server != NULL) {
		delete server;
	}
	if(ethernet != NULL) {
		delete ethernet;
	}
	if(iridium != NULL) {
		delete iridium;
	}
	if(gprs != NULL) {
		delete gprs;
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
	try {
		if(server) {
			server->signal_stop();
		}
		if(manager) {
			manager->signal_stop();
		}
	}
	catch(...) {
	}
}
