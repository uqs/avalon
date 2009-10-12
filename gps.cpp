/************************************************************************/
/*									                                    */
/*		               P R O J E K T    A V A L O N 			        */
/*								 	                                    */
/*	    gps.cpp		Producer for the gps-sensor. Captures all	        */
/*			        Data and brings it to the Store			            */
/*									                                    */
/*      Authors     Patrick Schwizer        patricsc@student.ethz.ch    */
/*                  Stefan Wismer           wismerst@student.ethz.ch    */
/*									                                    */
/************************************************************************/


#include <DDXStore.h>
#include <DDXVariable.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>
#include <DDXSerialComm.h>
#include <rtx/nmea.h>

#include "gps.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataV;


// Storage for the command line arguments
const char * varname = "gps";
const char * commport = "/dev/ttyACM0";
const char * producerHelpStr = "All Data coming from the GPS";

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
  RTX_GETOPT_END
};

void * gps_production_thread(void * dummy)
{
	gpsData gps;				//gps-struct
	int fd; 				//handle to the serial port
	RtxNMEA mesg;				//temporary storage for nmea-message
	char dir[82];				//for extracting South/North, East/West
	char fieldbuffer[82];			//storage for the first field of the nmea-message (message identifier)
    int deg;
    double minutes;
	fd = rtx_serial_open(
			commport,		//char *dev,
			19200, 			//int  	baud
			8,			//int  	databits,
			1,			//int  	stopbits,
			RTX_SERIAL_PARITY_NONE,	//RtxSerialParity  	parity,
			RTX_SERIAL_FLOW_NONE,	//RtxSerialFlow  	flow,
			RTX_SERIAL_MODEM_OFF,	//RtxSerialModem  	modem,
			0,			//int  	vmin,
			0			//int  	vtime
			);

	if (fd==-1){rtx_message("couldn't open gps-device");return 0;};
	while (1) {

		rtx_nmea_read(fd, &mesg);
		rtx_nmea_extract(&mesg,0,fieldbuffer);

		if(!strcasecmp(fieldbuffer,"GPVTG")){
			/* exract course */
			rtx_nmea_extract_double( &mesg, 1, &gps.course );

			/* exract speed in kmph */
			rtx_nmea_extract_double( &mesg, 7, &gps.speed_kmph );

			/* extract speed in knots */
			rtx_nmea_extract_double( &mesg, 5, &gps.speed_kn );
			continue;
		}

		if(!strcasecmp(fieldbuffer,"GPGGA")){

			//rtx_nmea_read_buffer( buffer, 82, &mesg );
			/* extract time
			   rtx_nmea_extract_double( &mesg, 1, &GPStime );
			   rtx_nmea_extract( &mesg, 1, charBuff );
			   sscanf(charBuff,"%2d%2d%lf", &(gps.time.hours), &(gps.time.mins), &(gps.time.secs) );
			   gps.time.hours = ((int)GPStime / 10000);*/

			/* extract latitude and longitude */
			rtx_nmea_extract(&mesg, 2, fieldbuffer);
            sscanf(fieldbuffer,"%2d%lf",&deg,&minutes);
            gps.position.latitude = deg + (minutes / 60.0);

			rtx_nmea_extract( &mesg, 3, dir );
			if( dir[0] == 'S' )  {
				gps.position.latitude = - gps.position.latitude;
			}

			rtx_nmea_extract(&mesg, 4, fieldbuffer);
            sscanf(fieldbuffer,"%3d%lf",&deg,&minutes);
            gps.position.longitude = deg + (minutes / 60.0);

			rtx_nmea_extract( &mesg, 5, dir );
			if( dir[0] == 'W' ) {
				gps.position.longitude = - gps.position.longitude;
			}
			/*extract number of satellites*/
			rtx_nmea_extract_int( &mesg, 7, &gps.sat );

			/* extract GPS fix indicator */
			rtx_nmea_extract_short( &mesg, 6, &gps.gpsFIX );

			/* exract GPS horizontal dilution of pression */
			rtx_nmea_extract_float( &mesg, 8, &gps.HDOP );

			/* extract altitude */
			rtx_nmea_extract_double( &mesg, 9, &gps.position.altitude );

			/* extract DGPS information */
			rtx_nmea_extract_short( &mesg, 13, &gps.dgpsage );
			rtx_nmea_extract_short( &mesg, 14, &gps.dgpsref );
			//rtx_message("longitude %f, latitude %f speed %f",gps.longitude,gps.latitude,gps.speed_kmph);

			dataV.t_writefrom(gps);
			continue;
		}
	};
	rtx_serial_close(fd);
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
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL,
					producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("store-producer", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the DataExample data type
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), gpsData));

	// Create output variable
	DOB(store.registerVariable(dataV,varname,"gpsData"));






	// Start the working thread
	DOP(th = rtx_thread_create ("thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
				RTX_THREAD_CANCEL_DEFERRED,
				gps_production_thread, NULL,
				NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Caught SIGINT/SIGQUIT, exiting ...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
	return (0);
}
