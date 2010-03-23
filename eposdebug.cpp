/************************************************************************/
/*									                                    */
/*		            P R O J E K T    A V A L O N 		                */
/*								 	                                    */
/*	    eposdebug.cpp   Test programm to test EPOS functionalities      */
/*									                                    */
/*	    Author          Stefan Wismer                                   */
/*	                    wismerst@student.ethz.ch                        */
/*									                                    */
/************************************************************************/


#include <stdlib.h>
#include <stdio.h>

// For command line arguments
#include <rtx/getopt.h>
#include <rtx/timer.h>

// For the Motors
#include "can/can.h"
#include "can/epos.h"

#include "avalon.h"
#include "SailMotor.h"
#include "RudderMotor.h"

/**
 * Storage for the command line arguments
 * */
const char * device = "/dev/ttyUSB2";
int reset = 0;
int brake = 0;
int homing = 0;
int poti = 0;
int debug = 1;
int serialnr = 0;
const char * consumerHelpStr = "Program to find errors in EPOS motor drivers";

/**
 * Command line arguments
 * */
RtxGetopt producerOpts[] = {
  {"port", "Name of the Port where the EPSO is",
   {
     {RTX_GETOPT_STR, &device, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"reset", "Shall the errors be reseted on start of the scritp?",
   {
     {RTX_GETOPT_INT, &reset, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"talk", "Should it talk to the user?",
   {
     {RTX_GETOPT_INT, &debug, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"serial", "Show serial number and exit",
   {
     {RTX_GETOPT_INT, &serialnr, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"brake", "Open or close the brake at startup (1 = open|0 = close)",
   {
     {RTX_GETOPT_INT, &brake, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"homing", "Conduct a homing operation on startup",
   {
     {RTX_GETOPT_INT, &homing, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"poti", "Display the value of the absolute encoder (only working with sail EPOS)",
   {
     {RTX_GETOPT_INT, &poti, ""},
     RTX_GETOPT_END_ARG
   }
  },
  RTX_GETOPT_END
};

int main(int argc, const char * argv[])
{
    int ret;
    SailMotor sailmotor;
    RudderMotor ruddermotor;

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, consumerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, consumerHelpStr);
		exit (1);
	}

    // Connect to motor (with or without reset)
    if(reset > 0)
    {
        if(debug > 0)
        {
            printf("Initialising Motor at %s with reset...\n", device);
        }
        ruddermotor.init(1, device);
    }
    else
    {
        if(debug > 0)
        {
            printf("Initialising Motor at %s without reset...\n", device);
        }
        can_init(device);
    }

    // If asked: print serial number
    if(serialnr != 0)
    {
        epos_get_serial_number(1);
        return 0;
    }

    /*
    // Set brake as asked by console...
    if(brake != 0)
    {
        if(debug > 0)
        {
            printf("Open Brake!!\n");
        }
        epos_set_brake_state(2,AV_SAIL_BRAKE_OPEN);
    }
    else
    {
        if(debug > 0)
        {
            printf("Close Brake!!\n");
        }
        epos_set_brake_state(2,AV_SAIL_BRAKE_CLOSE);
    }
    */

    if(homing != 0)
    {
        if(debug > 0)
        {
            printf("Going to start a homing operation...\n");
        }
        ruddermotor.conduct_homing(1);
    }

    if(poti != 0)
    {
        if(debug > 0)
        {
            printf("Going to ask for poti value...\n");
        }
        epos_ask_poti_position(AV_POTI_NODE_ID);
        epos_get_poti_position(AV_POTI_NODE_ID);

        // printf("Poti is now showing %d ticks\n", epos_read.node[1].
    }

    int i = 0;
    int mode = 0;
    while(i < 100)
    {
        epos_get_mode_of_operation_display(1);
        mode = epos_read.node[0].operation_mode_display;

        printf("Operation Mode after %d seconds is %d.\n",i/10, mode);
        
        rtx_timer_sleep(0.1);
        
        i++;
    }

    // Fehler ausgeben
	epos_get_error(1);

    // Fertig.
	can_close();

	return 0;
}
