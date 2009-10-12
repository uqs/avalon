

#include "MessageServer.h"

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

#include <iostream>

using namespace std;

// constants
const string logfilename = "mm.log";
const char RUN_AS_USER[] = "daemon";

// global
logstream info(cout.rdbuf());
logstream error(cout.rdbuf());
bool exit_flag = false;

// declarations
void detach(string logfilename);
void signal_handler(int signal);

int main(int argc, char *argv[])
{
	
	try {
		MessageServer server(2222);
		if(argc > 1 && strcmp(argv[1], "-f") == 0) {
			detach(logfilename);
			server.set_logfile(info.rdbuf());
		}
		signal(SIGINT,  signal_handler);
		signal(SIGTERM, signal_handler);
		
		server.run();

	}
	catch(string msg) {
		error << "Error " << msg << endl;
		return -1;
	}
	catch(...) {
		error << "unknown error" << endl;
		return -1;
	}
	return 0;
}


/**
 * Signal handler for termination signals.
 * Flags the signal to the server main loop on the first call and exits on the second.
 */
void signal_handler(int signal)
{
	// TODO: we probably shouldn't do I/O here.
	if(exit_flag) {
		error << "Caught second signal, dieing badly" << endl;
		exit(-1);	
	}
	info << "Received signal, waiting for servers to quit." << endl;
	MessageServer::signal_stop();
//	exit(0);	
}

/**
 * Signal handler for the parent process after forking child.
 */
void child_handler(int signum) {
	switch(signum) {
	    case SIGUSR1:
	    	info << "forked sucessfully, goodbye" << endl;
	    	exit(0);
	    	break;
	    case SIGCHLD:
	    case SIGALRM:
	    	error << "error forking to background" << endl;
	    	exit(EXIT_FAILURE);
	    	break;
	    default:
	    	break;
	}
}

/**
 * Detaches the current thread from any controlling tty. Forks to the background
 * and redirects standard in- and output to act as a daemon.
 * 
 * \exception std::string if anything goes wrong.
 */
void detach(string logfilename)
{
	pid_t pid, sid, parent;
//    int lfp = -1;

	/* already a daemon */
	if ( getppid() == 1 ) {
		return;
	}
	
	/* Create the lock file as the current user */
	// no locking for now, opening the network port will suffice.
/*	if ( lockfile && lockfile[0] ) {
		lfp = open(lockfile,O_RDWR|O_CREAT,0640);
		if ( lfp < 0 ) {
			syslog( LOG_ERR, "unable to create lock file %s, code=%d (%s)",
			        lockfile, errno, strerror(errno) );
			exit(EXIT_FAILURE);
		}
	}
*/

	/* Trap signals that we expect to recieve in parent process */
	signal(SIGCHLD,child_handler);
	signal(SIGUSR1,child_handler);
	signal(SIGALRM,child_handler);
	
	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		throw(string("unable to fork daemon: ") + strerror(errno) );
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		info << "forked to background with pid " << pid << endl;
	    /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
	       for two seconds to elapse (SIGALRM).  pause() should not return. */
	    alarm(2);
	    pause();
	
		throw(string("while waiting for child process to take over"));
	}
	
	/* At this point we are executing as the child process */
	parent = getppid();
	
	/* Cancel certain signals */
	signal(SIGCHLD,SIG_DFL); /* A child process dies */
	signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM,SIG_DFL); /* Die on SIGTERM */
	
	/* Change the file mode mask */
	umask(0);
	
	filebuf logfile;
	// find log directory: /var/log/ if root, otherwise the current directory
	if(geteuid() == 0) {
		logfile.open(("/var/log/" + logfilename).c_str(), ios::out|ios::app);
	}
	else {
		logfile.open(logfilename.c_str(), ios::out|ios::app);
	}
	if(!logfile.is_open()) {
		throw("error opening logfile");
	}
	info.rdbuf(&logfile);
	error.rdbuf(&logfile);
	
	/* Redirect standard files to /dev/null */
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stderr);
	freopen("/dev/null", "w", stdout);
	
	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		throw(string("unable to create a new session: ") + strerror(errno));
	}
	
	/* Change the current working directory.  This prevents the current
	   directory from being locked; hence not being able to remove it. */
	if ((chdir("/")) < 0) {
		throw(string("unable to change directory to /") + strerror(errno) );
	}
	
	/* Tell the parent process that we are okay */
	kill( parent, SIGUSR1 );
	
	/* Drop user if there is one, and we were run as root */
	if( getuid() == 0 || geteuid() == 0 ) {
	    struct passwd *pw = getpwnam(RUN_AS_USER);
	    if( pw ) {
	    	info << "setting user to " << RUN_AS_USER << endl;
	        setuid( pw->pw_uid );
	    }
	}
	return;
}
