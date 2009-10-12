/************************************************************************/
/*								                                    	*/
/*      		       P R O J E K T    A V A L O N 			        */
/*									                                    */
/*	    sailmain.cpp	Sail-Driver Main Program			            */
/*									                                    */
/*	    Author  	    Stefan Wismer               			        */
/*                      wismerst@student.ethz.ch                        */
/*									                                    */
/************************************************************************/

/**
 *	This reads the target angles form the Store and sets it on the Rudder-EPOS.
 *	Then the actual Angle of the Rudders are returned.
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
#include "include/can.h"
#include "include/ddxjoystick.h"

#include "sail-target.h"
#include "SailMotor.h"
#include "Sailstate.h"
#include "flags.h"
#include "ports.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataSail;
DDXVariable dataSailState;
DDXVariable dataFlags;
DDXVariable dataPorts;

SailMotor motor;
Ports ports;
// char commport[99] = "auto\0";

/**
 * Storage for the command line arguments
 * */
const char * varname = "sail";
const char * varname_state = "sailstate";
const char * varname_ports = "ports";
const char * commport = "auto";
const char * consumerHelpStr = "Sail-Motor EPOS-driver";
char device[99] = "";

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
  {"port", "Devicename",
   {
     {RTX_GETOPT_STR, &commport, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"where_port", "Store Variable where the Ports are",
   {
     {RTX_GETOPT_STR, &varname_ports, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"statename", "Store-Variable where the feedback is written",
   {
     {RTX_GETOPT_STR, &varname_state, ""},
     RTX_GETOPT_END_ARG
   }
  },
  RTX_GETOPT_END
};

	
/**
 * Working thread, wait the data, transform them and write them again
 * */
void * sailmain_thread(void * dummy)
{
	sailTarget sail;
	Sailstate sailState = {0,0};

	int ret = 0;
    int demand_speed = 0;
    int demand_position = 0;
    int speed = 0;
    int current = 0;
    int position = 0;
    int feedback = 0;
    bool brake = true;
    bool reset_active = false;
    int num_rounds = 0;

    // Initialize sailState (Set everything to zero)
	dataSailState.t_writefrom(sailState);

	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (1) {
            dataSailState.t_readto(sailState,0,0);
			dataSail.t_readto(sail,0,0);

            // check if errors are present
            epos_read.node[AV_SAIL_NODE_ID].req_reset = 0;
            // rtx_timer_sleep(0.1);
            ret = epos_check_errors(AV_SAIL_NODE_ID);
            if (ret != 0)
            {
                rtx_message("RESET REQUIRED!!!!\n");
                motor.init(device);
                epos_read.node[AV_SAIL_NODE_ID-1].req_reset = 0;
                errors_present = 0;
            }

			// Go there!!
			motor.move_to_angle(sail.degrees, sailState.ref_sail, feedback, num_rounds);
			//sailState.degrees_sail =  feedback / AV_SAIL_TICKS_PER_DEGREE;
            sailState.degrees_sail = remainder((feedback / (AV_SAIL_TICKS_PER_DEGREE)),360.0);
			dataSailState.t_writefrom(sailState);

            // Ask for Some of the Data and display
            epos_get_actual_velocity(AV_SAIL_NODE_ID);
            epos_get_actual_position(AV_SAIL_NODE_ID);
            epos_get_current_actual_value(AV_SAIL_NODE_ID);
            epos_get_velocity_demand_value(AV_SAIL_NODE_ID);
            epos_get_position_demand_value(AV_SAIL_NODE_ID);
            speed = epos_read.node[AV_SAIL_NODE_ID - 1].actual_velocity;
            demand_speed = epos_read.node[AV_SAIL_NODE_ID - 1].demand_velocity;
            position = epos_read.node[AV_SAIL_NODE_ID - 1].actual_position;
            current = epos_read.node[AV_SAIL_NODE_ID - 1].actual_current;
            demand_position = epos_read.node[AV_SAIL_NODE_ID - 1].demand_position;

            if(speed == 0 && demand_speed == 0 && demand_position == position)
            {
                brake = true;
            }
            else
            {
                brake = false;
            }

#ifdef AV_SAILMAIN_CONTROLCENTER
            // Control Center !! =)
            printf("|  Speed\t\t%d\trpm\t|\n", speed);
            printf("|  Demand Speed\t\t%d\trpm\t|\n",demand_speed);
            printf("|  Position\t\t%d\tticks\t|\n",position);
            printf("|  Demand Position\t%d\tticks\t|\n",demand_position);
            printf("|  Current\t\t%d\tmA\t|\n",current);
            printf("|\t\t\t\t\t|\n");
            if(brake)
            {
                printf("|  Brake\tclose\t\t\t|\n");
            }
            else
            {
                printf("|  Brake\topen\t\t\t|\n");
            }
            printf("-----------------------------------------\n");
#endif // AV_SAILMAIN_CONTROLCENTER

            if(brake)
            {
                epos_set_brake_state(AV_SAIL_NODE_ID, AV_SAIL_BRAKE_CLOSE);
            }
            else
            {
                epos_set_brake_state(AV_SAIL_NODE_ID, AV_SAIL_BRAKE_OPEN);
            }

			// Reset Options (if requested by the Remote-Controller)
            if(sail.reset_request == 1 && !reset_active)
            {
                rtx_message("Resetting sail ... ");
                sailState.ref_sail = sailState.degrees_sail;
                reset_active = true;
				// Bring Encoder and reference values to Store
				dataSailState.t_writefrom(sailState);
            }
            else
            {
                reset_active = false;
            }

	

        } else if (dataSail.hasTimedOut()) {
			// Timeout. Probably no joystick connected.
			rtx_message("Timeout while reading Sail-Target data.\n");

		} else {
			// Something strange happend. Critical Error. 
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}	
	}
	rtx_message("Thread reaches end... ");
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
	rtx_main_init ("Sail-Motor Driver Main", RTX_ERROR_STDERR);
   
    printf("Port von Kommandozeile: %s\n", commport);

    // Open the store
	DOB(store.open());

    // Register Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailTarget));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Sailstate));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Ports));

	// Connect to sail-target-variable, and create variables for the target-data
	DOB(store.registerVariable(dataSail, varname, "sailTarget"));
	DOB(store.registerVariable(dataSailState, varname_state, "Sailstate"));
	DOB(store.registerVariable(dataPorts, varname_ports, "Ports"));

    dataPorts.t_readto(ports,0,0);
    if (!strcmp(commport, "auto")) {
        sprintf(device, "/dev/ttyUSB%d", ports.sail);
    }

    printf("Going to open %s\n", device);

	// Initialisation of the Motor 
	motor.init(device);

	// Start the working thread
	DOP(th = rtx_thread_create ("Sailmotor driver thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0, 
								RTX_THREAD_CANCEL_DEFERRED,
								sailmain_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down Sail-Motor Driver...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// TODO CP: Add motor cleanup code here

	// The destructors will take care of cleaning up the memory
	return (0);
}
