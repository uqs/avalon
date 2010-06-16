/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	windsensor.cpp	  Producer for the Windsensor. Captures all     */
/*                        Data and brings it to the Store		        */ 
/*									                                    */
/*	    Author  	      Stefan Wismer			                        */
/*                        wismerst@student.ethz.ch                      */
/*									                                    */
/************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// This files are always included
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>
#include <math.h>

// This files are included just in this specific case.
#include <rtx/serial.h>
#include <rtx/nmea.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "windsensor.h"

/***/
// Macro Error handling for all functions
/***/
// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}
// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}
// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}

// Global Variables
DDXStore store;
DDXVariable dataWind;

// Storage for the command line arguments
const char * varname = "wind";
const char * commport = "/dev/windsensor";
const char * producerHelpStr = "All Data coming from the Windsensor";
bool talk = false;
int iTalk = 0;


// Command line arguments processing
RtxGetopt producerOpts[] = {
  {"name", "name of the variable to register",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"port", "name of the comm port to use",
   {
     {RTX_GETOPT_STR, &commport, ""},
     RTX_GETOPT_END_ARG
   }
  },

  {"talk", "Do we want the tool to tell us where we are?",
   {
     {RTX_GETOPT_INT, &iTalk, ""},
     RTX_GETOPT_END_ARG
   }
  },

  RTX_GETOPT_END
};

void * wind_thread(void * dummy)
{
  WindData wind;			// Windsensor Data Structure
  int fd;				// Handle to the Serial Port
  int ret;				// Return Value
  RtxNMEA mesg;				// For temp-saving the whole NMEA-sentence
  char headerBuffer[82];		// For temp-saving the header of the sentence

  // Open Serial Port
  fd = rtx_serial_open(commport, 4800, 8, 1, RTX_SERIAL_PARITY_NONE, RTX_SERIAL_FLOW_NONE,
			RTX_SERIAL_MODEM_OFF, 0, 0);

  if(fd == -1) {
    rtx_error("Command rtx_serial_open failed. fd is -1.");
    return 0;
  }

  // Set Values to out-dated.
  wind.uptodate = false;

  while (1) {
	  // Get Next NMEA-sentence and header from device
	  rtx_nmea_read(fd, &mesg);
	  rtx_nmea_extract(&mesg, 0, headerBuffer);

	  // Decide what kind of message we have
	  if (strcasecmp(headerBuffer,"WIXDR")==0) {
		  // If not: Update Voltage and Temperature
		  rtx_nmea_extract_double(&mesg, 2, &wind.temperature);
		  rtx_nmea_extract_double(&mesg, 10, &wind.voltage);
		  wind.uptodate =true;
	  } else if (strcasecmp(headerBuffer,"WIMWV")==0) {

		// Wind direction
		ret = rtx_nmea_extract_double(&mesg, 1, &(wind.direction));
		if(ret == -1) {
			  rtx_message("Error in rtx_nmea_extract_int when extracting wind direction.");
		}

		wind.direction = remainder(wind.direction, 360.0);

		// Wind speed
		ret = rtx_nmea_extract_double(&mesg, 3, &(wind.speed));
		if(ret == -1) {
		 rtx_message("Error in rtx_nmea_extract_double when extracting wind speed.");
		}
		// If complete nonsence: do nothing and continue
		//
		 // Debug Message
		if (talk)
			{if(wind.uptodate) {
				rtx_message("Dir: %d Deg, Speed: %f Knots, Temp: %f °C, U = %f V", wind.direction, wind.speed, wind.temperature, wind.voltage);
			}
			else {
				rtx_message("Dir: %d Deg, Speed: %f Knots", wind.direction, wind.speed);
			}
		;}

		  // Write the data to the store
		  dataWind.t_writefrom(wind);
	  } else {
		rtx_message("windsensor not on this port: %c",commport);
		  // we're not interested here
		  continue;
	  }
  }

  // Clean up
  rtx_serial_close(fd);

  return NULL;
}


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
	rtx_main_init ("Windsensor-Driver Main", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the DataExample data type
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindData));

	// Create output variable
	DOB(store.registerVariable(dataWind,varname,"WindData"));

	// Parse Commandline input
	if(iTalk == 1)
	{
		talk = true;
	}

  // Start the working thread
  DOP(th = rtx_thread_create ("Windsensor Driver Thread", 0,
			      RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
			      RTX_THREAD_CANCEL_DEFERRED,
			      wind_thread, NULL,
			      NULL, NULL));

  // Wait for Ctrl-C
  DOC (rtx_main_wait_shutdown (0));
  rtx_message_routine ("Ctrl-C detected. Shutting down Windsensor-Producer...");

  // Terminating the thread
  rtx_thread_destroy_sync (th);

  // The destructors will take care of cleaning up the memory
  return (0);
}


