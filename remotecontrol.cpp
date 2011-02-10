/************************************************************************/
/*								                                    	*/
/*      		       P R O J E K T    A V A L O N 			        */
/*									                                    */
/*	    remotecontrol.cpp	Converts Joystick Signals into Sail and     */
/*	                        Rudder Position Signals                     */
/*									                                    */
/*	    Author  	        Stefan Wismer			                    */
/*                          wismerst@student.ethz.ch                    */
/*									                                    */
/************************************************************************/

/**
 *	This converts the Joy-Stick signals into sail- and rudder angles
 *
 **/

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

// Specific Things
#include "include/ddxjoystick.h"


#include <DDXStore.h>
#include <DDXVariable.h>

#include "sail-target.h"
#include "rudder-target.h"
#include "flags.h" 
#include "Sailstate.h"
#include "Rudderstate.h"
#include "imu.h"
#include "desired_course.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataJoystick;
DDXVariable dataRudder;
DDXVariable dataSail;
DDXVariable dataFlags;
DDXVariable dataRcFlags;
DDXVariable dataRudderStateLeft;
DDXVariable dataRudderStateRight;
DDXVariable dataSailState;
DDXVariable dataImu;
DDXVariable dataDesiredHeading;

/**
 * Storage for the command line arguments
 * */
const char * varname_joy = "joystick";
const char * varname_rudder = "rudder";
const char * varname_sail = "sail";
const char * varname_flags = "flags";
const char * varname_rcflags = "rcflags";
const char * varname_sailstate = "sailstate";
const char * varname_rudderstateleft = "rudderstateleft";
const char * varname_rudderstateright = "rudderstateright";
const char * varname_imu = "imu";
const char * varname_desiredheading = "desiredheading";
bool talk = false;
int iTalk = 0;
const char * producerHelpStr = "Remote-Control Interface";

/**
 * Command line arguments
 *
 * */
RtxGetopt producerOpts[] = {
	{"inname", "Store-Variable where the data fromt the joystick is",
		{
			{RTX_GETOPT_STR, &varname_joy, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"ruddername", "Store Variable where the rudder-angle target is written",
		{
			{RTX_GETOPT_STR, &varname_rudder, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"sailname", "Store Variable where the sail-angle target is written",
		{
			{RTX_GETOPT_STR, &varname_sail, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"flagsname", "Store Variable where the safety flags are written",
		{
			{RTX_GETOPT_STR, &varname_flags, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"talk", "Do we want the tool to tell us where we are?",
		{
			{RTX_GETOPT_INT, &iTalk, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"sailstatename", "Where the sail feedback is written",
		{
			{RTX_GETOPT_STR, &varname_sailstate, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"rudderleftstatename", "Where the left rudder feedback is written",
		{
			{RTX_GETOPT_STR, &varname_rudderstateleft, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"rcflagsname", "Store-Variable where the rc-Flags are",
		{
			{RTX_GETOPT_STR, &varname_rcflags, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"rudderrightstatename", "Where the right rudder feedback is written",
		{
			{RTX_GETOPT_STR, &varname_rudderstateright, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"imuname", "Store Variable where the imuData is",
		{
			{RTX_GETOPT_STR, &varname_imu, ""},
			RTX_GETOPT_END_ARG
		}
	},
	{"desiredheadingname", "Store Variable where the desired heading is",
		{
			{RTX_GETOPT_STR, &varname_desiredheading, ""},
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
	DDX_JOYSTICK joystick;
	rudderTarget rudder = {0.0, 0.0, 0, 0};
	sailTarget sail = {0.0, 0};
	Sailstate sailState;
	Rudderstate rudderstateleft, rudderstateright;
	Flags flags;
	rcFlags rcflags;
	imuData imu;
	DesiredHeading desired_heading;
	bool pressed = false, stop_pressed = false, sailreset_pressed = false, hdgch_pressed = false;
	bool leftreset_pressed = false, rightreset_pressed = false, sailing_pressed = false;
	bool rc_pressed = false, energysaving_pressed = false, oceansailing_pressed = false;
	bool emergency = false, motionstop = false;
	float sail_corrected, rudderleft_corrected, rudderright_corrected; // To have the correct output

	// Initialize to be in remote-control when starting up
	emergency = false;
	motionstop = false;
	rcflags.man_in_charge = AV_FLAGS_MIC_REMOTECONTROL;
	rcflags.sailorstate_requested = AV_FLAGS_ST_IDLE;
	dataSailState.t_readto(sailState,0,0);
	sail.degrees = sailState.degrees_sail;
	dataSail.t_writefrom(sail);

	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (dataJoystick.t_readto(joystick,10.0,1)) {

			dataFlags.t_readto(flags,0,0);
			dataSailState.t_readto(sailState,0,0);
			dataRudderStateLeft.t_readto(rudderstateleft,0,0);
			dataRudderStateRight.t_readto(rudderstateright,0,0);
			dataImu.t_readto(imu,0,0);

			// Set Remotecontrol Flag and reset timeout
			rcflags.joystick_timeout = 0;

			// Rudder steering via x-axis
			if(joystick.buttons[0]){
				rudder.degrees_left = (joystick.axes[0]/AV_JOYSTICK_RES_X)*AV_MAX_RUDDER_ANGLE;
				rudder.degrees_right = rudder.degrees_left;
			}
			else {
				rudder.degrees_left = (joystick.axes[0]/AV_JOYSTICK_RES_X)*AV_JOYSTICK_RUDDER_FINE;
				rudder.degrees_right = rudder.degrees_left;
			}

			// 	Sail steering via 4/5 buttons
			if(joystick.buttons[3] && flags.man_in_charge == AV_FLAGS_MIC_REMOTECONTROL)
			{
				sail.degrees = remainder((sail.degrees - AV_SAILSTICK_4_5_SENSITIVITY),360.0);
			}
			if(joystick.buttons[4] && flags.man_in_charge == AV_FLAGS_MIC_REMOTECONTROL)
			{
				sail.degrees = remainder((sail.degrees + AV_SAILSTICK_4_5_SENSITIVITY),360.0);
			}

			// 	Desired_heading steering via 4/5 buttons in sailing mode (only
			// 	if navigator is idle
			if(joystick.buttons[3] && flags.man_in_charge == AV_FLAGS_MIC_SAILOR && flags.navi_state == AV_FLAGS_NAVI_IDLE && flags.state != AV_FLAGS_ST_DOCK)
			{
				dataDesiredHeading.t_readto(desired_heading,0,0);
				desired_heading.heading -= AV_DESHEADSTICK_4_5_SENSITIVITY;
				desired_heading.heading = remainder(desired_heading.heading, 360.0);
				dataDesiredHeading.t_writefrom(desired_heading);
			}
			if(joystick.buttons[4] && flags.man_in_charge == AV_FLAGS_MIC_SAILOR && flags.navi_state == AV_FLAGS_NAVI_IDLE && flags.state != AV_FLAGS_ST_DOCK)
			{
				dataDesiredHeading.t_readto(desired_heading,0,0);
				desired_heading.heading += AV_DESHEADSTICK_4_5_SENSITIVITY;
				desired_heading.heading = remainder(desired_heading.heading, 360.0);
				dataDesiredHeading.t_writefrom(desired_heading);
			}

			//--------------------------------------------------------------------------------------------------------------------
			//Increas / Decrease desired heading 60 degrees (to test step responses of the sailor) [inserted 100620 by gbu]
#if 1
			if(joystick.buttons[3] && joystick.buttons[0] && flags.state == AV_FLAGS_ST_DOCK && !hdgch_pressed)
			{
                hdgch_pressed = true;
				dataDesiredHeading.t_readto(desired_heading,0,0);
				desired_heading.heading -= 60.0;
				desired_heading.heading = remainder(desired_heading.heading, 360.0);
				dataDesiredHeading.t_writefrom(desired_heading);
			}
            else if(joystick.buttons[4] && joystick.buttons[0] && flags.state == AV_FLAGS_ST_DOCK && !hdgch_pressed)
			{
                hdgch_pressed = true;
				dataDesiredHeading.t_readto(desired_heading,0,0);
				desired_heading.heading += 60.0;
				desired_heading.heading = remainder(desired_heading.heading, 360.0);
				dataDesiredHeading.t_writefrom(desired_heading);
			}
            else if(!joystick.buttons[0] && flags.state == AV_FLAGS_ST_DOCK)
            {
                hdgch_pressed = false;
            }
#endif
			//---------------------------------------------------------------------------------------------------------------------
			//
#if 0			//homing of rudders and sail is now done autmatically
			//
			// Request Rudder reset (buttons 2 and 8 respectivly 9)
			if(joystick.buttons[1] && joystick.buttons[7] && !leftreset_pressed)
			{
				rudder.resetleft_request = 1;
				leftreset_pressed = true;
			}
			else
			{
				rudder.resetleft_request = 0;
				leftreset_pressed = false;
			}

			if(joystick.buttons[1] && joystick.buttons[8] && !rightreset_pressed)
			{
				rudder.resetright_request = 1;
				rightreset_pressed = true;
			}
			else
			{
				rudder.resetright_request = 0;
				rightreset_pressed = false;
			}

			// Request Sail reset (buttons 2 and 10)
			if(joystick.buttons[1] && joystick.buttons[9] && !sailreset_pressed)
			{
				sail.reset_request = 1;
				sail.degrees = 0;           // Prevent uncontrolled Motion...
				sailreset_pressed = true;
			}
			else
			{
				sail.reset_request = 0;
				sailreset_pressed = false;
			}
#endif

			// Emergency Stop (Motion of boom)
			if(joystick.buttons[10] && !stop_pressed)
			{
				stop_pressed = true;
				motionstop = true;
				emergency = false;
				rtx_message("Stopping...");
				// Set current position as target
				dataSailState.t_readto(sailState,0,0);
				sail.degrees = sailState.degrees_sail;
				dataSail.t_writefrom(sail);
				rcflags.man_in_charge = AV_FLAGS_MIC_NOONE;
				rcflags.sailorstate_requested = AV_FLAGS_ST_IDLE;
				rcflags.autonom_navigation = 0;
			}
			else
			{
				if(!joystick.buttons[10])
				{
					stop_pressed = false;
				}
			}

			// Emergency Stop (activate Sail to Wind)
			if(joystick.buttons[1] && joystick.buttons[2] && !pressed)
			{
				pressed = true;
				emergency = true;
				motionstop = false;
				rcflags.man_in_charge = AV_FLAGS_MIC_SAILOR;
				rcflags.sailorstate_requested = AV_FLAGS_ST_DOCK;
				desired_heading.heading = imu.attitude.yaw;
				dataDesiredHeading.t_writefrom(desired_heading);
				rcflags.autonom_navigation = 0;
			}
			else
			{
				if(!(joystick.buttons[1] && joystick.buttons[2]))
				{
					pressed = false;
				}
			}

			// Activate MaxEnergySaving
			if(joystick.buttons[1] && joystick.buttons[3] && !energysaving_pressed)
			{
				energysaving_pressed = true;
				emergency = false;
				motionstop = false;
				rcflags.man_in_charge = AV_FLAGS_MIC_SAILOR;
				rcflags.sailorstate_requested = AV_FLAGS_ST_MAXENERGYSAVING;
				rcflags.autonom_navigation = 0;
			}
			else
			{
				if(!(joystick.buttons[1] && joystick.buttons[3]))
				{
					energysaving_pressed = false;
				}
			}

			// Normal Sailing without navigator
			if(joystick.buttons[1] && joystick.buttons[5] && !sailing_pressed)
			{
				sailing_pressed = true;
				emergency = false;
				motionstop = false;
				rcflags.man_in_charge = AV_FLAGS_MIC_SAILOR;
				rcflags.sailorstate_requested = AV_FLAGS_ST_NORMALSAILING;
				rcflags.autonom_navigation = 0;
				// for testing: make current heading = desired heading
				desired_heading.heading = imu.attitude.yaw;
				dataDesiredHeading.t_writefrom(desired_heading);
			}
			else
			{
				if(!(joystick.buttons[1] && joystick.buttons[5]))
				{
					sailing_pressed = false;
				}
			}

			// Ocean Sailing (Normal Sailing with navigator)
			if(joystick.buttons[1] && joystick.buttons[4] && !oceansailing_pressed)
			{
				oceansailing_pressed = true;
				emergency = false;
				motionstop = false;
				rcflags.man_in_charge = AV_FLAGS_MIC_SAILOR;
				rcflags.sailorstate_requested = AV_FLAGS_ST_NORMALSAILING;
				rcflags.autonom_navigation = 1;
				// for testing: make current heading = desired heading
				desired_heading.heading = imu.attitude.yaw;
				dataDesiredHeading.t_writefrom(desired_heading);
			}
			else
			{
				if(!(joystick.buttons[1] && joystick.buttons[4]))
				{
					oceansailing_pressed = false;
				}
			}

			// Back to Remote-Control from whereever we are
			if(joystick.buttons[1] && joystick.buttons[6] && !rc_pressed)
			{
				rc_pressed = true;
				emergency = false;
				motionstop = false;
				rcflags.man_in_charge = AV_FLAGS_MIC_REMOTECONTROL;
				rcflags.sailorstate_requested = AV_FLAGS_ST_IDLE;
				rcflags.autonom_navigation = 0;
				// Set current position as target
				dataSailState.t_readto(sailState,0,0);
				sail.degrees = sailState.degrees_sail;
			}
			else
			{
				if(!(joystick.buttons[1] && joystick.buttons[6]))
				{
					rc_pressed = false;
				}
			}

			// Debug Messages (Talk-mode only...)
			if(talk)
			{
				if (emergency)
				{
					rtx_message("Emergency Stop active. Everything is on halt!");
				}
				else
				{
					sail_corrected = sail.degrees + sailState.ref_sail;
					rudderleft_corrected = rudder.degrees_left + rudderstateleft.ref_rudder;
					rudderright_corrected = rudder.degrees_right + rudderstateright.ref_rudder;
					rtx_message("Rudders are at %3d deg and %3d deg and sail is at %4d deg.", (int)rudder.degrees_left, (int)rudder.degrees_right, (int)sail_corrected);
				}
			}

			// Set Flags!
			if (emergency)
			{
				rcflags.emergency_stop = 1;
			}
			else
			{
				rcflags.emergency_stop = 0;
			}

			if (motionstop)
			{
				rcflags.motion_stop= 1;
			}
			else
			{
				rcflags.motion_stop = 0;
			}



			// Reset requested sailor state, once state has been reached
			if (flags.state == rcflags.sailorstate_requested)
			{
				rcflags.sailorstate_requested = 0;
			}

			// Bring to store
			if(flags.man_in_charge == 2)        // Write only if we are in charge of the ship!!
			{
				dataRudder.t_writefrom(rudder);
				dataSail.t_writefrom(sail);
			}
			dataRcFlags.t_writefrom(rcflags);

		} else if (dataJoystick.hasTimedOut()) {
			// Timeout. Probably no joystick connected.
			// Action: inform user and tell Hendrik's program that something is wrong
			rtx_message("Timeout while reading joystick data. Is the joystick connected?\n");
			rcflags.joystick_timeout = 1;
			if(flags.man_in_charge == AV_FLAGS_MIC_SAILOR) //if joystick timeout while sailing --> go into MAXENERGYSAVING
			{
				rcflags.man_in_charge = AV_FLAGS_MIC_SAILOR;
				rcflags.sailorstate_requested = AV_FLAGS_ST_MAXENERGYSAVING;
				/* if GB's program is running, don't do MaxEnergySaving! */
				if(flags.autonom_navigation == 1)
				{
					rcflags.sailorstate_requested = AV_FLAGS_ST_NORMALSAILING;
				}
			}
			else // Motion Stop!!
			{
				motionstop = true;
				rtx_message("Stopping...");
				// Set current position as target
				dataSailState.t_readto(sailState,0,0);
				sail.degrees = sailState.degrees_sail;
				dataSail.t_writefrom(sail);
				rcflags.man_in_charge = AV_FLAGS_MIC_NOONE;
				rcflags.sailorstate_requested = AV_FLAGS_ST_IDLE;
			}
			dataRcFlags.t_writefrom(rcflags);

		} 
		else 
		{
			// Something strange happend. Critical Error. 
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}	
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
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Remote-Control Interace Main", RTX_ERROR_STDERR);

	// Parse Commandline input
	if(iTalk == 1)
	{
		talk = true;
	}

	// Open the store
	DOB(store.open());

	// Register the Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rudderTarget));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailTarget));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rcFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DDX_JOYSTICK));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Sailstate));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Rudderstate));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DesiredHeading));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));

	// Connect to joystick-variable, and create variables for the target-data
	DOB(store.registerVariable(dataJoystick, varname_joy, "DDX_JOYSTICK"));
	DOB(store.registerVariable(dataRudder, varname_rudder, "rudderTarget"));
	DOB(store.registerVariable(dataSail, varname_sail, "sailTarget"));
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
	DOB(store.registerVariable(dataRcFlags, varname_rcflags, "rcFlags"));
	DOB(store.registerVariable(dataSailState, varname_sailstate, "Sailstate"));
	DOB(store.registerVariable(dataRudderStateLeft, varname_rudderstateleft, "Rudderstate"));
	DOB(store.registerVariable(dataRudderStateRight, varname_rudderstateright, "Rudderstate"));
	DOB(store.registerVariable(dataDesiredHeading, varname_desiredheading, "DesiredHeading"));
	DOB(store.registerVariable(dataImu, varname_imu, "imuData"));

	// Start the working thread
	DOP(th = rtx_thread_create ("Remote-Control Interface thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0, 
				RTX_THREAD_CANCEL_DEFERRED,
				translation_thread, NULL,
				NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down Remote-Control Interface...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
	return (0);
}
