/************************************************************************/
/*									*/
/*		       P R O J E K T    A V A L O N 			*/
/*									*/
/*	ruddercalibrate.h	Calibration routines for the Rudders	*/
/*									*/
/*	Last Change		March 17, 2009; Stefan Wismer		*/
/*									*/
/************************************************************************/


/**
 *  This program allows controlling the two rudders independently and setting the 0°- Mark 
 *  for each of the Motor-Encoders
 *
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include <DDXStore.h>
#include <DDXVariable.h>

// Specific Things
#include "include/ddxjoystick.h"

#include "include/can.h"
#include "include/epos.h"

#include "rudder-target.h"
#include "RudderMotor.h"

#define ID 0x0001

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataRudder;
DDXVariable dataJoystick;

/**
 * Storage for the command line arguments
 * */
const char * varname = "rudder";
const char * varname_joy = "joystick";
const char * commport = "none";
const char * consumerHelpStr = "Rudder-Motor Calibration Tool";
unsigned int canid_left = 0x0001;
unsigned int canid_right = 0x0001;

/**
 * Command line arguments
 * */
RtxGetopt producerOpts[] = {
  {"inname", "Store-Variable where the target angle is",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"joystick", "Store-Variable where the joystick-data is",
   {
     {RTX_GETOPT_STR, &varname_joy, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"port", "Device where the EPOS is plugged",
   {
     {RTX_GETOPT_STR, &commport, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"canid_left", "CAN id of the left Rudder-EPOS",
   {
     {RTX_GETOPT_INT, &canid_left, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"canid_right", "CAN id of the right Rudder-EPOS",
   {
     {RTX_GETOPT_INT, &canid_right, ""},
     RTX_GETOPT_END_ARG
   }
  },
};

void * rudderconfig_thread(void * dummy)
{
	rudderTarget rudder;
	RudderMotor motor;
	DDX_JOYSTICK joystick;

	int stupid = 0;

	// INIT-SEQUENCE EPOS-MOTORS
	motor.init(1, commport);
	
	while (1) {
		// Read the next data available, or wait at most 10 seconds
		if (dataJoystick.t_readto(joystick,10.0,1)) {
			// Here comes the main thing
			if(joystick.buttons[7])
			{
				// Move left rudder
				rudder.degrees_left += AV_JOYSTICK_RUDDER_SENSITIVITY * joystick.axes[0];
				motor.move_to_angle(1, rudder.degrees_left, stupid);
				break; 
			}	
			else if(joystick.buttons[8])
			{
				// Move right rudder
				rudder.degrees_right += AV_JOYSTICK_RUDDER_SENSITIVITY * joystick.axes[0];
				motor.move_to_angle(2, rudder.degrees_right, stupid);
				break; 
			}

			if(joystick.buttons[0])		// ATTENTION: Check which button that was...
			{
				// Confirm 0°-Position
				if(joystick.buttons[7])
				{
					// Confirm left
					motor.calibrate(1);
					break;
				}	
				else if(joystick.buttons[8])
				{
					// Confirm right
					motor.calibrate(2);
					break;
				}
			}
				
				
	
		} else if (dataRudder.hasTimedOut()) {
			// Timeout. Probably no joystick connected.
			rtx_message("Timeout while reading Rudder-Target data.\n");

		} else {
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
main (int argc, char * argv[])
{
	RtxThread * th;
	int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, consumerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, consumerHelpStr);
		exit (1);
	}
	rtx_main_init ("Rudder Calibration Routine", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Connect to rudder-target-variable and joystick data for the button
	DOB(store.registerVariable(dataRudder, varname, "rudderTarget"));
	DOB(store.registerVariable(dataJoystick, varname_joy, "DDX_JOYSTICK"));


	// Start the working thread
	DOP(th = rtx_thread_create ("Rudder-Motor Calibration Thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0, 
								RTX_THREAD_CANCEL_DEFERRED,
								rudderconfig_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down calibration routine");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
