/************************************************************************/
/*																		*/
/*      		       P R O J E K T    A V A L O N						*/
/*																		*/
/*	    flag-checker.cpp    ALL GENERAL CONSTANTS.					 	*/
/*																		*/
/*	    Last Change	        April 02, 2009; Stefan Wismer				*/
/*																	  	*/
/************************************************************************/

/**
 *	This sets all the general flags based on the individual flags
 *
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "flags.h" 

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataFlags;
DDXVariable dataRcFlags;
DDXVariable dataSailorFlags;
DDXVariable dataNaviFlags;
DDXVariable skipperFlagData;

/**
 * Storage for the command line arguments
 * */
const char * varname_flags = "flags";
const char * varname_rcflags = "rcflags";
const char * varname_sailorflags = "sailorflags";
const char * varname_naviflags = "naviflags";
const char * varname_skipperflags = "skipperflags";
const char * producerHelpStr = "Flags-Checker who sets all the general Flags";

/**
 * Command line arguments
 * */

RtxGetopt producerOpts[] = {
  {"flagsname", "Store Variable where the flags are",
   {
     {RTX_GETOPT_STR, &varname_flags, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"rcflagsname", "Store Variable where the RC-Flags are",
   {
     {RTX_GETOPT_STR, &varname_rcflags, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"sailorflags", "Store Variable where the Sailor-Flags are",
   {
     {RTX_GETOPT_STR, &varname_sailorflags, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"naviflags", "Store Variable where the Navi-Flags are",
   {
     {RTX_GETOPT_STR, &varname_naviflags, "NaviFlags"},
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
	Flags flags;
	rcFlags rcflags;
    sailorFlags sailorflags;
    NaviFlags naviflags;
    SkipperFlags skipperflags;

    /* Initialization */
    dataFlags.t_readto(flags,0,0);
    dataRcFlags.t_readto(rcflags,0,0);
    dataSailorFlags.t_readto(sailorflags,0,0);
    dataNaviFlags.t_readto(naviflags,0,0);
    skipperFlagData.t_readto(skipperflags,0,0);

    // General Flags:
    flags.man_in_charge = AV_FLAGS_MIC_REMOTECONTROL;
    flags.state = AV_FLAGS_ST_IDLE;
    flags.state_requested = 0;
    flags.sail_direction = 0;
    flags.sailor_no_tack_or_jibe = 0;
    flags.joystick_timeout = 0;
    flags.navi_state = AV_FLAGS_NAVI_IDLE;
    flags.navi_index_call = 0;
    flags.navi_index_answer = 0;

    flags.autonom_navigation = 0;
    flags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;

    // RC-Flags:
    rcflags.emergency_stop = 0;
    rcflags.motion_stop = 0;
    rcflags.joystick_timeout = 0;
    rcflags.sailorstate_requested = 0;
    rcflags.man_in_charge = AV_FLAGS_MIC_REMOTECONTROL;
    rcflags.autonom_navigation = 0;

    // Sailor Flags:
    sailorflags.state = 0;
    sailorflags.sail_direction = 0;
    sailorflags.no_tack_or_jibe = 0;

    // Navigation Flags:
    
    //naviflags.navi_state = AV_FLAGS_NAVI_NORMALNAVIGATION; //shoudn't make a difference, gets filled further down!!
    naviflags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION; //shoudn't make a difference, gets filled further down!!
    naviflags.navi_index_call = 0;
    naviflags.navi_index_answer = 0;

    //Skipper Flags:
    skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;

    //writing to store the initialized flags:
    skipperFlagData.t_writefrom(skipperflags);
    dataFlags.t_writefrom(flags);
    dataRcFlags.t_writefrom(rcflags);
    dataSailorFlags.t_writefrom(sailorflags);
    dataNaviFlags.t_writefrom(naviflags);

	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (1) {
			dataFlags.t_readto(flags,10.0,1);
            dataRcFlags.t_readto(rcflags,0,0);
            dataSailorFlags.t_readto(sailorflags,0,0);
            dataNaviFlags.t_readto(naviflags,0,0);
            skipperFlagData.t_readto(skipperflags,0,0);

            // Reset state_requested if state has been reached:
            if(flags.state == flags.state_requested)
            {
                flags.state_requested = 0;
            }

            switch(rcflags.man_in_charge)
            {
                case AV_FLAGS_MIC_NOONE:
                    flags.man_in_charge = AV_FLAGS_MIC_NOONE;
                    break;
                case AV_FLAGS_MIC_SAILOR:
                    flags.man_in_charge = AV_FLAGS_MIC_SAILOR;
                    break;
                case AV_FLAGS_MIC_REMOTECONTROL:
                    flags.man_in_charge = AV_FLAGS_MIC_REMOTECONTROL;
                    break;
                default:
                    flags.man_in_charge = AV_FLAGS_MIC_NOONE;
            }

            // Set joystick_timeout
            
            if(rcflags.joystick_timeout == 0)
            {
                flags.joystick_timeout = 0;
            }
            else
            {
                flags.joystick_timeout = 1;
            }


            //write the naviflags.indexes to flags:
            
            flags.navi_index_call = naviflags.navi_index_call;
            flags.navi_index_answer = naviflags.navi_index_answer;



            switch(skipperflags.global_locator)
            {
                case AV_FLAGS_GLOBALSK_LOCATOR:
                    flags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                    break;
                case AV_FLAGS_GLOBALSK_TRACKER:
                    flags.global_locator = AV_FLAGS_GLOBALSK_TRACKER;
                    break;
                case AV_FLAGS_GLOBALSK_CLOSING:
                    flags.global_locator = AV_FLAGS_GLOBALSK_CLOSING;
                    break;
                default:
                    flags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
            }

            switch(rcflags.autonom_navigation)
            {
                case 0:
                    flags.autonom_navigation = 0;
                    break;
                case 1:
                    flags.autonom_navigation = 1;
                    break;
                default:
                    flags.autonom_navigation = 0;
            }

            // Process sailor state transitions (just a pro forma action)
            switch(sailorflags.state)
            {
                case 0:
                    break; // stay in current state
                case AV_FLAGS_ST_IDLE:
                    flags.state = AV_FLAGS_ST_IDLE;
                    break;
                case AV_FLAGS_ST_DOCK:
                    flags.state = AV_FLAGS_ST_DOCK;
                    break;
                case AV_FLAGS_ST_NORMALSAILING:
                    flags.state = AV_FLAGS_ST_NORMALSAILING;
                    break;
                case AV_FLAGS_ST_TACK:
                    flags.state = AV_FLAGS_ST_TACK;
                    break;
                case AV_FLAGS_ST_JIBE:
                    flags.state = AV_FLAGS_ST_JIBE;
                    break;
                case AV_FLAGS_ST_UPWINDSAILING:
                    flags.state = AV_FLAGS_ST_UPWINDSAILING;
                    break;
                case AV_FLAGS_ST_DOWNWINDSAILING:
                    flags.state = AV_FLAGS_ST_DOWNWINDSAILING;
                    break;
                case AV_FLAGS_ST_MAXENERGYSAVING:
                    flags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                    break;
                case AV_FLAGS_ST_HEADINGCHANGE:
                    flags.state = AV_FLAGS_ST_HEADINGCHANGE;
                    break;
                default:
                    rtx_message("sailor_transitions requested illegal state transition...");
            }

            // set sail_direction (just pro forma)
            switch(sailorflags.sail_direction)
            {
                case AV_FLAGS_SAIL_DIR_NOPREFERENCE:
                    flags.sail_direction = AV_FLAGS_SAIL_DIR_NOPREFERENCE;
                    break;
                case AV_FLAGS_SAIL_DIR_ZERO:
                    flags.sail_direction = AV_FLAGS_SAIL_DIR_ZERO;
                    break;
                case AV_FLAGS_SAIL_DIR_FRONT:
                    flags.sail_direction = AV_FLAGS_SAIL_DIR_FRONT;
                    break;
                default:
                    rtx_message("sailor_transitions requested illegal sail direction for HEADINGCHANGE...");
                    flags.sail_direction = AV_FLAGS_SAIL_DIR_NOPREFERENCE;
            }

            // Set no_tack_or_jibe (just pro forma)
            switch(sailorflags.no_tack_or_jibe)
            {
                case 0:
                    flags.sailor_no_tack_or_jibe = 0;
                    break;
                case 1:
                    flags.sailor_no_tack_or_jibe = 1;
                    break;
                default:
                    rtx_message("sailorflags.no_tack_or_jibe has illegal state...");
                    flags.sailor_no_tack_or_jibe = 0;
            }

            // Process sailor state requests from Remote Control (just a pro forma action)
            switch(rcflags.sailorstate_requested)
            {
                case 0:
                    break; // dont do anything
                case AV_FLAGS_ST_IDLE:
                    flags.state_requested = AV_FLAGS_ST_IDLE;
                    break;
                case AV_FLAGS_ST_DOCK:
                    flags.state_requested = AV_FLAGS_ST_DOCK;
                    break;
                case AV_FLAGS_ST_NORMALSAILING:
                    flags.state_requested = AV_FLAGS_ST_NORMALSAILING;
                    break;
                case AV_FLAGS_ST_TACK:
                    flags.state_requested = AV_FLAGS_ST_TACK;
                    break;
                case AV_FLAGS_ST_JIBE:
                    flags.state_requested = AV_FLAGS_ST_JIBE;
                    break;
                case AV_FLAGS_ST_UPWINDSAILING:
                    flags.state_requested = AV_FLAGS_ST_UPWINDSAILING;
                    break;
                case AV_FLAGS_ST_DOWNWINDSAILING:
                    flags.state_requested = AV_FLAGS_ST_DOWNWINDSAILING;
                    break;
                case AV_FLAGS_ST_MAXENERGYSAVING:
                    flags.state_requested = AV_FLAGS_ST_MAXENERGYSAVING;
                    break;
                case AV_FLAGS_ST_HEADINGCHANGE:
                    flags.state_requested = AV_FLAGS_ST_HEADINGCHANGE;
                    break;
                default:
                    rtx_message("remote control requested illegal state transition...");
            }
            
            switch(rcflags.autonom_navigation)
            {
                case 0:
                    {
                        flags.navi_state = AV_FLAGS_NAVI_IDLE;
                        break;
                    }
                case 1:
                    {
                        //Navigator Calling:
                        switch(naviflags.navi_state)
                        {
                            case 0:
                                rtx_message("something wrong with the navi_state, should not go through");
                                break; // dont do anything
                            case AV_FLAGS_NAVI_IDLE: //notfall, this case should not get used!
                                flags.navi_state = AV_FLAGS_NAVI_IDLE;
                                break;
                            case AV_FLAGS_NAVI_NEWCALCULATION:
                                flags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION;
                                break;
                            case AV_FLAGS_NAVI_NORMALNAVIGATION:
                                flags.navi_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                                break;
                            case AV_FLAGS_NAVI_GOAL_APPROACH:
                                flags.navi_state = AV_FLAGS_NAVI_GOAL_APPROACH;
                                break;
                            default:
                                flags.navi_state = AV_FLAGS_NAVI_IDLE;
                                rtx_message("navi-state requested illegal...");
                        }
                        break;
                    }
                default:
                    flags.navi_state = AV_FLAGS_NAVI_IDLE;
            }


            // write flags data to store
		    dataFlags.t_writefrom(flags);
            // Do not write to any other flags here. Only one program per
            // flags-variable!!

		} else if (dataFlags.hasTimedOut()) {
			// Timeout.
			// Inform user 
			rtx_message("Timeout while reading flags data");
		} 
		else 
		{
			// Something strange happend. Critical Error. 
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}	
        rtx_timer_sleep(0.1);
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
main (int argc, char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Flag-checker", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rcFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailorFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), SkipperFlags));


	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
	DOB(store.registerVariable(dataRcFlags, varname_rcflags, "rcFlags"));
	DOB(store.registerVariable(dataSailorFlags, varname_sailorflags, "sailorFlags"));
	DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));
	DOB(store.registerVariable(skipperFlagData, varname_skipperflags, "SkipperFlags"));

	// Start the working thread
    DOP(th = rtx_thread_create ("Flag Checker Thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down Flag Checker...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
