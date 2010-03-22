// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "sail-target.h"
#include "Sailstate.h"
#include "flags.h" 

// things for the shell command function
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataSailState;
DDXVariable dataSail;
DDXVariable dataFlags;

/**
 * This function executes shell commands
 * */
int RunCommand(const char *strCommand);

/**
 * Storage for the command line arguments
 * */
const char * producerHelpStr = "This will restart sailmain if it doesn't work anymore";
const char * varname_sailtarget = "sail";
const char * varname_sailstate = "sailstate";
const char * varname_flags = "flags";

/**
 * Command line arguments
 * */
RtxGetopt producerOpts[] = {
    {"sailname", "Store Variable where the sail-angle target is written",
        {
            {RTX_GETOPT_STR, &varname_sailtarget , ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"sailstatename", "Store Variable where sail state is",
        {
            {RTX_GETOPT_STR, &varname_sailstate, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"flagsname", "Store Variable where the flags are",
        {
            {RTX_GETOPT_STR, &varname_flags, ""},
            RTX_GETOPT_END_ARG
        }
    },
    RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{
    sailTarget sail = {0.0, 0};
    Sailstate sailstate;
    double last_sailstate;
	Flags flags;

    int count = 0;
    bool firsttime = true;
    int runcommand_errstate = 0;
#ifdef AV_SAILMAIN_KEEPALIVE_LOGFILE
    FILE * logfile = fopen("sailmain_keepalive.txt","w+");
#endif

	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (1)
		{
			dataSailState.t_readto(sailstate,0,0);
            dataSail.t_readto(sail,0,0);
            dataFlags.t_readto(flags,0,0);

            if((flags.man_in_charge != AV_FLAGS_MIC_NOONE) // someone is in charge
                    && (fabs(sailstate.degrees_sail - sail.degrees) > 5.0) // sail is in the wrong place
                    && (sailstate.degrees_sail == last_sailstate) // sail is not moving
                    && !firsttime) // last_sailstate is valid
            {
                count++;
                if(count > 5) // sail has not moved for at least 5 seconds
                {
                    count = 0;
                    runcommand_errstate = RunCommand("killall sailmain");
                    rtx_timer_sleep(1.0);
                    runcommand_errstate = RunCommand("./sailmain");
                    if(runcommand_errstate == 0)
                    {
                        rtx_message("Restarted sailmain!!");
#ifdef AV_SAILMAIN_KEEPALIVE_LOGFILE
                        fprintf(logfile, "Restarted sailmain... \n");
#endif
                    }
                    else
                    {
                        rtx_message("Error while trying to restart sailmain!!");
#ifdef AV_SAILMAIN_KEEPALIVE_LOGFILE
                        fprintf(logfile, "Error while trying to restart sailmain!! \n");
#endif
                    }
                }
            }
            firsttime = false;
            last_sailstate = sailstate.degrees_sail;
            rtx_timer_sleep(1.0);
		}
		else if (dataSailState.hasTimedOut()) {
			rtx_message("Timeout while reading SailState-Data \n");
        }
        else if (dataFlags.hasTimedOut()) {
            rtx_message("Timeout while reading flags data");
		} 
		else {
			// Something strange happend. Critical Error.
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}
	}
    fclose (logfile);
	return NULL;
}

// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}


/* A little function that executes shell commands */
int RunCommand(const char *strCommand)
{
	int iForkId, iStatus;
	iForkId = vfork();
	if (iForkId == 0)	// This is the child 
	{
		iStatus = execl("/bin/sh","sh","-c", strCommand, (char*) NULL);
		exit(iStatus);	// We must exit here, 
				// or we will have multiple
				// mainlines running...  
	}
	else if (iForkId > 0)	// Parent, no error
	{
		iStatus = 0;
	}
	else	// Parent, with error (iForkId == -1)
	{
		iStatus = -1;
	}
	return(iStatus);
}


int main (int argc, char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Sailmain keepalive interface Main", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailTarget));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Sailstate));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));


	// Connect to variables, and create variables for the target-data
    DOB(store.registerVariable(dataSail, varname_sailtarget, "sailTarget"));
    DOB(store.registerVariable(dataSailState, varname_sailstate, "Sailstate"));
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));

	// Start the working thread
    DOP(th = rtx_thread_create ("Sailmain keepalive thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down sailmain_keepalive...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
