
#include <string>
#include <stdlib.h>
#include <rtx/signal.h>
#include <rtx/error.h>
#include <rtx/message.h>
#include <rtx/main.h>
#include <rtx/thread.h>
#include <rtx/time.h>

static std::string command;

void * working_thread(void *)
{
	while (1) {
		int r;
		r = system(command.c_str());
		sleep(1);
	}
	return NULL;
}


// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}

int main(int argc,char *argv[])
{

	RtxThread * th = NULL;
	int i;
	for (i=1;i<argc-1;i++) {
		command+=argv[i];
		command+=" ";
	}
	command += argv[i];

	DOC(rtx_main_init("keepalive",0));
	rtx_signal_block_realtime();

	// Start the working thread
	DOP(th = rtx_thread_create ("thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
				RTX_THREAD_CANCEL_DEFERRED,
				working_thread, NULL,
				NULL, NULL));


	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message ("Caught SIGINT/SIGQUIT, exiting ...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// std::string killit("killall");
	// system((killit + " " + argv[1]).c_str());
	// sleep(1);
	// system((killit + " -9 " + argv[1]).c_str());


	return 0;
}




