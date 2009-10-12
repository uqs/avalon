
#include "Logstream.h"
#include "Exception.h"

#include "utils.h"

#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

extern Logstream info;
extern Logstream error;

/*
 * Some code shamelessly copied from different online sources, such as
 * http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
 * http://www.lowtek.com/sockets/select.html
 */


/**
 * Signal handler for the parent process after forking child.
 */
extern "C" void child_handler(int signum) {
	switch(signum) {
		case SIGUSR1:
			info << "forked sucessfully, goodbye" << std::endl;
			exit(0);
			break;
		case SIGCHLD:
		case SIGALRM:
			error << "error forking to background" << std::endl;
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
void detach(std::string logfilename, std::filebuf &logfile)
{
	pid_t pid, sid, parent;

	// already a daemon
	if ( getppid() == 1 ) {
		return;
	}
	
	// Trap signals that we expect to recieve in parent process
	signal(SIGCHLD,child_handler);
	signal(SIGUSR1,child_handler);
	signal(SIGALRM,child_handler);
	
	// Fork off the parent process
	pid = fork();
	if (pid < 0) {
		throw(Exception("unable to fork daemon", errno) );
	}
	// If we got a good PID, then we can exit the parent process.
	if (pid > 0) {
		info << "forked to background with pid " << pid << std::endl;
		// Wait for confirmation from the child via SIGTERM or SIGCHLD, or
		// for two seconds to elapse (SIGALRM).  pause() should not return.
		alarm(5);
		pause();
	
		throw(Exception("while waiting for child process to take over"));
	}
	
	// At this point we are executing as the child process
	parent = getppid();
	
	// Cancel certain signals
	signal(SIGCHLD,SIG_DFL); // A child process dies
	signal(SIGTSTP,SIG_IGN); // Various TTY signals
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);	// not to interfere with Threads
	signal(SIGHUP, SIG_IGN); // Ignore hangup signal
	signal(SIGTERM,SIG_DFL); // Die on SIGTERM
	
	// Change the file mode mask
	umask(0);
	
	// find log directory: /var/log/ if root, otherwise the current directory
	if(geteuid() == 0) {
		logfile.open(("/var/log/" + logfilename).c_str(), std::ios::out|std::ios::app);
	}
	else {
		logfile.open(logfilename.c_str(), std::ios::out|std::ios::app);
	}
	if(!logfile.is_open()) {
		throw(Exception("error opening logfile"));
	}
	info.rdbuf(&logfile);
	error.rdbuf(&logfile);
	
	
	// Redirect standard files to /dev/null 
	info << "closing standard file streams" << std::endl;
	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stderr);
	freopen("/dev/null", "w", stdout);
	
	// Create a new SID for the child process
	info << "creating new SID" << std::endl;
	sid = setsid();
	if (sid < 0) {
		throw(Exception("unable to create a new session", errno));
	}
	
	// Change the current working directory.  This prevents the current
	// directory from being locked; hence not being able to remove it.
	info << "changing directory to /" << std::endl;
	if ((chdir("/")) < 0) {
		throw(Exception("unable to change directory to /", errno) );
	}
	
	// Tell the parent process that we are okay
	info << "notify parent: ";
	if(kill( parent, SIGUSR1 ) == -1) {
		throw(Exception("unable to notify parent", errno) );
	}
	info << "done" << std::endl;
	
	return;
}
