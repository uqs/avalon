/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	porttracker.cpp     Checks the Port Numbers for the EPOS        */ 
/*									                                    */
/*	    Author      	    Stefan Wismer           			        */
/*                          wismerst@student.ethz.ch                    */
/*									                                    */
/************************************************************************/

/**
 * This checks all the serial ports if it can find an EPOS,
 * reads the serial number and writes to the store which
 * EPOS is which.
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
#include "include/epos.h"
#include "include/can.h"

#include "ports.h"
#include "RudderMotor.h"

/**
 * Some Function prototype
 * */
bool file_exists(const char * filename);

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataPorts;

// These are not acutally a rudder motor, just a generic motor.
RudderMotor motor;

/**
 * Storage for the command line arguments
 * */
const char * varname = "ports";
const char * consumerHelpStr = "This checks which port is which EPOS";

/**
 * Command line arguments
 *
 * */
RtxGetopt producerOpts[] = {
  {"portsname", "Store-Variable where the ports are",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
  RTX_GETOPT_END
};

	
/**
 * Working thread, does nothing. 
 * */
void * rudderdrive_thread(void * dummy)
{
	/*
    rudderTarget rudder;
	Rudderstate rudderstate;

	int feedback = 0;
    bool left_pressed = false, right_pressed = false;

	rudderstate.ref_rudder = 0;
	dataRudderState.t_writefrom(rudderstate);

	while (1) {
		// Read the next data available, or wait at most 10 seconds
		if (dataRudder.t_readto(rudder,10.0,1))
        {
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

    */

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
	// RtxThread * th;
	int ret;
    int nofound = 0;
    char device[99];
    int found_so_far = 0;
    Ports ports = {0,0,0};

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, consumerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, consumerHelpStr);
		exit (1);
	}
	rtx_main_init ("Porttracker Main", RTX_ERROR_STDERR);
	
	// Open the store
	DOB(store.open());

	// Register Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Ports));

	// Connect to rudder-target-variable, and create variables for the target-data
	DOB(store.registerVariable(dataPorts, varname, "Ports"));

	// INIT-SEQUENCE EPOS-MOTORS
    rtx_message("EPOS - Serial-Port Tracker...");

    int i = 0;
    while(i < AV_NUMBER_OF_SERIAL_PORTS)
    {
        if(nofound > 10)
        {
            break;
        }
        
        sprintf(device, "/dev/ttyUSB%d", i);
        printf("Testing device %s...\n", device);

        if(!file_exists(device))
        {
            i++;
            nofound++;
            continue;
        }
        
        if(!motor.init(1, device))
        {
            printf("%s has no EPOS\n",device);
        }
        else
        {
            printf("%s has a EPOS\n",device);

            switch(found_so_far)
            {
            case 0:
                printf("Rudder left found at %s\n",device);
                dataPorts.t_readto(ports,1.0,1);
                ports.rudderright = i;
                dataPorts.t_writefrom(ports);
                break;

            case 1:
                printf("Rudder right found at %s\n",device);
                dataPorts.t_readto(ports,1.0,1);
                ports.rudderleft = i;
                dataPorts.t_writefrom(ports);
                break;

            case 2:
                printf("Sail found at %s\n",device);
                dataPorts.t_readto(ports,1.0,1);
                ports.sail = i;
                dataPorts.t_writefrom(ports);
                break;

            default:
                printf("ERROR - something went wrong!!");
                // TODO: Process this error!
                return 0;
            }
            found_so_far++;
            
        }
        i++;
    }
    
    // Bring to store (all at once)
    printf("Writing to store...\n");

    //////////////
    // This Program needs no working thread, so this section becomes
    // pointles...
    // ///////////

    /*
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

    */
	
	// The destructors will take care of cleaning up the memory
	return (0);
}

bool file_exists(const char * filename)
{
    if (FILE * file = fopen(filename, "r"))
    {
        fclose(file);
        return true;
    }
    return false;
}
