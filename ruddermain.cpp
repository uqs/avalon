/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	ruddermain.cpp	Contains the Programm for Rudder Steering.	    */
/*									                                    */
/*	    Author  	    Stefan Wismer			                        */
/*                      wismerst@student.ethz.ch                        */
/*									                                    */
/************************************************************************/

/**
 *	This reads the target angles form the Store and sets it on the Rudder-EPOS
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

// Specific Things
#include "eposlib-test/epos.h"
// #include "include/can.h"
#include "include/ddxjoystick.h"

#include "rudder-target.h"
#include "RudderMotor.h"
#include "Rudderstate.h"
#include "ports.h"

#define ID 0x0001

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataRudder;
DDXVariable dataRudderState;
DDXVariable dataPorts;

RudderMotor motor;
Ports ports;
char commport[99] = "auto"; 

/**
 * Storage for the command line arguments
 * */
const char * varname = "rudder";
const char * varname_ports = "ports";
const char * varname_state_left = "rudderstateleft";
const char * varname_state_right = "rudderstateright";
const char * varname_state = "rudderstateright";            // Make it log enou
const char * side = "none";
const char * consumerHelpStr = "Rudder-Motor EPOS-driver";
unsigned int rudderside = AV_RUDDER_LEFT;

/**
 * Command line arguments
 *
 * */
RtxGetopt producerOpts[] = {
  {"inname", "Store-Variable where the target angle is",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"where_port", "Store-Variable where the port angle is",
   {
     {RTX_GETOPT_STR, &varname_ports, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"port", "Devicename (default: /dev/ttyUSB0 for left and /dev/ttyUBS1 for right)",
   {
     {RTX_GETOPT_STR, &commport, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"side", "'left' or 'right'",
   {
     {RTX_GETOPT_STR, &side, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"statename_left", "Store-Variable where left rudder-feedback is written to",
   {
     {RTX_GETOPT_STR, &varname_state_left, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"statename_right", "Store-Variable where right rudder-feedback is written to",
   {
     {RTX_GETOPT_STR, &varname_state_right, ""},
     RTX_GETOPT_END_ARG
   }
  },
  RTX_GETOPT_END
};

	
/**
 * Working thread, wait the data, transform them and write them again
 * */
void * rudderdrive_thread(void * dummy)
{
	rudderTarget rudder;
	Rudderstate rudderstate;

	int feedback = 0;
    int speed = 0;
    int current = 0;
    int ret = 0;
    bool left_pressed = false, right_pressed = false;

	rudderstate.ref_rudder = 0;
	dataRudderState.t_writefrom(rudderstate);

	while (1) {
		// Read the next data available, or wait at most 10 seconds
		if (dataRudder.t_readto(rudder,10.0,1))
        {
            // check if errors are present
            epos_read.node[1].req_reset = 0;
            //rtx_timer_sleep(0.1);
            ret = epos_check_errors(1);
            if (ret != 0)
            {
                rtx_message("RESET REQUIRED!!!!\n");
                motor.init(1, commport);
                epos_read.node[1].req_reset = 0;
                errors_present = 0;
            }

			// Get old Encoder values and joystick data from the store
			// dataRudderState.t_readto(rudderState,0,0);
			dataRudderState.t_readto(rudderstate,0,0);
		
			// Go there!!
			if(rudderside == AV_RUDDER_LEFT) {		
				motor.move_to_angle(1, rudder.degrees_left, rudderstate.ref_rudder, feedback);
				rudderstate.degrees_rudder =  feedback / AV_RUDDER_TICKS_PER_DEGREE;
			}
			else if(rudderside == AV_RUDDER_RIGHT) {
				motor.move_to_angle(1, rudder.degrees_right, rudderstate.ref_rudder, feedback);
				rudderstate.degrees_rudder =  feedback / AV_RUDDER_TICKS_PER_DEGREE;
			}

            // Ask for Some of the Data and display
            epos_get_actual_velocity(1);
            epos_get_current_actual_value(1);
            speed = epos_read.node[0].actual_velocity;
            current = epos_read.node[0].actual_current;

            /* 
            // Debuging stuff...
            printf("Speed is actually about %d rpm.\n", speed);
            printf("Current is %d mA.\n", current);
            printf("----------------\n");
            */
			
            // Reset Options
            if(rudder.resetleft_request && rudderside == AV_RUDDER_LEFT && !left_pressed)
            {
                rtx_message("Resetting left rudder ... ");
                rudderstate.ref_rudder = rudderstate.degrees_rudder;
                left_pressed = true;
            }
            else if(!rudder.resetleft_request && rudderside == AV_RUDDER_LEFT)
            {
                left_pressed = false;
            }
            
            if(rudder.resetright_request && rudderside == AV_RUDDER_RIGHT && !right_pressed)
            {
                rtx_message("Resetting right rudder ... ");
                rudderstate.ref_rudder = rudderstate.degrees_rudder;
                right_pressed = true;
            }
            else if(!rudder.resetleft_request && rudderside == AV_RUDDER_RIGHT)
            {
                right_pressed = false;
            }

			// Bring Encoder and reference values to Store
            dataRudderState.t_writefrom(rudderstate);

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

int main (int argc, char * argv[])
{
	RtxThread * th;
	int ret;

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, consumerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, consumerHelpStr);
		exit (1);
	}
	rtx_main_init ("Rudder-Motor Driver Main", RTX_ERROR_STDERR);
	
    // Open the store
	DOB(store.open());

	// Register Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rudderTarget));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Rudderstate));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Ports));

	// Connect to rudder-target-variable, and create variables for the target-data
	DOB(store.registerVariable(dataRudder, varname, "rudderTarget"));
	DOB(store.registerVariable(dataRudderState, varname_state, "Rudderstate"));
    DOB(store.registerVariable(dataPorts, varname_ports , "Ports"));

    // get Port-Info
    dataPorts.t_readto(ports,0,0);

	// Parse input-Arguments
	bool found = false;
	if (!strcmp(side, "left")) {
		rudderside = AV_RUDDER_LEFT;
		if (!strcmp(commport, "auto")) {
			sprintf(commport, "/dev/ttyUSB%d", ports.rudderleft);
            varname_state = varname_state_left;
		}
		found = true;
	}
	else if (!strcmp(side, "right")) {
		rudderside = AV_RUDDER_RIGHT;
		if (!strcmp(commport, "auto")) {
			sprintf(commport, "/dev/ttyUSB%d", ports.rudderright);
            varname_state = varname_state_right;
		}
		found = true;
	}
	else if (!found) {
		rtx_message("No rudderside specified. Terminating Programm...");
		return (-1);
	}

	
	// INIT-SEQUENCE EPOS-MOTORS
    rtx_message("Initializing rudders... ");
	if(!motor.init(rudderside, commport))
    {
        rtx_message("Something failed... Exiting.\n");
        return 0;
    }
    motor.conduct_homing(1);

	// Start the working thread
	DOP(th = rtx_thread_create ("Ruddermotor driver thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0, 
								RTX_THREAD_CANCEL_DEFERRED,
								rudderdrive_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down Rudder-Motor Driver...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);
	
	// TODO CP: add motor cleanup code here 

	// The destructors will take care of cleaning up the memory
	return (0);
}
