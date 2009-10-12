/**
 * This example is a data producer, it generate data every seconds. It could
 * also be waiting on the serial port or other input.
 *
 * */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "system-state.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataV;

/**
 * Storage for the command line arguments
 * */
double period = 5.0;
int readsensors = 0;
const char * varname = "system";
const char * producerHelpStr = "DDX system monitor";

/**
 * Command line arguments
 *
 * */
RtxGetopt producerOpts[] = {
  {"name", "name of the variable to register",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"period", "update period",
   {
     {RTX_GETOPT_DOUBLE, &period, "period"},
     RTX_GETOPT_END_ARG
   }
  },
  {"sensors", "read sensor state using lmsensors",
   {
     {RTX_GETOPT_SET, &readsensors, "sensors"},
     RTX_GETOPT_END_ARG
   }
  },
  RTX_GETOPT_END
};

void read_uptime(SystemState *state)
{
	FILE * fp = popen("uptime | sed -e 's/[a-z,]//g'","r");
	if (fp == NULL) {
		return;
	} else {
		unsigned int uph,upm;
		fscanf(fp, " %*d:%*d:%*d %d:%d %d : %f %f %f",
				&uph,&upm,&state->nuser,
				&state->loadavg1, &state->loadavg5, &state->loadavg15);
		state->uptime = uph*3600. + (upm*60.);
		pclose(fp);
	}
}

void read_mem(SystemState *state)
{
	FILE * fp = popen("free | sed -e 's=[A-Za-z:/+-]==g'","r");
	if (fp == NULL) {
		return;
	} else {
		char buffer[256];
		fgets(buffer,255,fp); // skip first line.
		fgets(buffer,255,fp); // get total from here
		sscanf(buffer, " %d ", &state->totalram);
		fgets(buffer,255,fp); // get used and free from here
		sscanf(buffer, " %d %d ", &state->usedram, &state->freeram);
		pclose(fp);
	}
}

void read_sensors(SystemState *state)
{
	FILE * fp = popen("sensors | egrep '(fan1|temp1|temp4):'","r");
	if (fp == NULL) {
		return;
	} else {
		char buffer[256];
		fgets(buffer,255,fp); // skip first line.
		sscanf(buffer," %*s %f",&state->fanrpm);
		fgets(buffer,255,fp); // skip first line.
		sscanf(buffer," %*s %f",&state->cputemp);
		fgets(buffer,255,fp); // skip first line.
		sscanf(buffer," %*s %f",&state->gputemp);
		pclose(fp);
	}
}

int done = 0;
void * production_thread(void * dummy)
{
	SystemState data;
	while (!done) {
		// Fill the data structure
		memset(&data,0,sizeof(SystemState));

		rtx_thread_setcancel(RTX_THREAD_CANCEL_DISABLE);
		read_uptime(&data);
		read_mem(&data);
		if (readsensors) {
			read_sensors(&data);
		}
		rtx_thread_setcancel(RTX_THREAD_CANCEL_DEFERRED);
		if (done) break;

		// Write the data to the store
		dataV.t_writefrom(data);
		rtx_timer_sleep(period);
	}
	return NULL;
}

		
// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}} 

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;} 

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;} 

int
main (int argc, const char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, 
			       producerHelpStr)) == -1) {
        RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
        exit (1);
    }
    rtx_main_init ("store-producer", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the DataExample data type
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), SystemState));

	// Create output variable
	DOB(store.registerVariable(dataV,varname,"SystemState"));

	// Start the working thread
    DOP(th = rtx_thread_create ("thfunc", 0,
				 RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0, 
				 RTX_THREAD_CANCEL_DEFERRED,
				 production_thread, NULL,
				 NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Caught SIGINT/SIGQUIT, exiting ...");

	done = 1;
	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
 


