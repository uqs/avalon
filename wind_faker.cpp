/**
 *
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

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

#include "windcleaner.h"
#include "windsensor.h"
#include "imu.h"
#include "flags.h"
#include "waypoints.h"
#include "destination.h"
#include "desired_course.h"


/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWindClean; //to get clean wind Data
DDXVariable dataBoat; //do get imu-Data
DDXVariable dataFlags;
DDXVariable dataNaviFlags;
DDXVariable destinationData;
DDXVariable destinationStruct;
DDXVariable headingData; //the curr heading will be written here; sailor needs that!!!


/**
 * Prototypes for utility functions
 * */
//int sign(int i);
//int sign(float i);


/**
 * Storage for the command line arguments
 * */


const char * varname2 = "cleanwind";
const char * varname4 = "imu";
const char * varname_flags = "flags";
const char * varname_naviflags = "naviflags";
const char * varname_destData = "destData";
const char * varname_destStruct = "destStruct";
const char * varname_course = "desiredheading";


const char * producerHelpStr = "skipper help-string";

/**
 * Command line arguments   //has yet to be completed
 *
 * */
RtxGetopt producerOpts[] = {
  {"cleanedWind", "Store Variable where the cleaned Wind data is read from",
   {
     {RTX_GETOPT_STR, &varname2, "WindCleanData"},
     RTX_GETOPT_END_ARG
   }
  },
#if 0
  {"destination","Variable where the calculated navi-waypoints are stored",
   {
     {RTX_GETOPT_STR, &varname_dest, "Waypoints"},
     RTX_GETOPT_END_ARG
   }
  },
#endif
  {"imuData", "Store Variable where the imuData is written",
   {
     {RTX_GETOPT_STR, &varname4, "imuData"},
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
    WindCleanData cleanedwind;
    imuData boatData;
    //Waypoints waypoints;
    //Flags generalflags;
    NaviFlags naviflags;
    //LakeTransformation transformation;
    DestinationData destination;
    DesiredHeading desiredHeading; //course that goes to HE

    // double simheading;
    double simspeed = 30*0.51444;
    // double headingError = 0;             //in percent
    double heading_average;
    double headingHistory[8];

    double current_pos_longitude, current_pos_latitude; //already transformed and in meters
    double next_pos_longitude, next_pos_latitude; //already transformed and in meters
    //the four vectors to be needed are:
    //double dist_solltrajectory, dist_next;
    //double area_soll, area_next;
    int first_time = 1;         //to determine if its the skipper just has been started and no wyp has yet been calculated!

    //fill the headingHistory:

    headingData.t_readto(desiredHeading,0,0);
    for(int i=0; i<8; i++)
    {
        headingHistory[i] = desiredHeading.heading;
    }


	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (1)
		{
            destinationData.t_readto(destination,0,0);
            dataWindClean.t_readto(cleanedwind,0,0);
            headingData.t_readto(desiredHeading,0,0);
            dataBoat.t_readto(boatData,0,0);
            dataNaviFlags.t_readto(naviflags,0,0);


            cleanedwind.speed_long = 10; //knots
            cleanedwind.global_direction_real = 150; //45; //remainder((rand() % 360),360); //generate winddirection;
            cleanedwind.global_direction_app = cleanedwind.global_direction_real;
            cleanedwind.global_direction_real_long = cleanedwind.global_direction_real;
            cleanedwind.bearing_real = remainder((cleanedwind.global_direction_real - boatData.attitude.yaw),360); //45; //remainder((rand() % 360),360); //generate winddirection;
            cleanedwind.bearing_app = cleanedwind.bearing_real;
            dataWindClean.t_writefrom(cleanedwind);
        }

            //has to be modified:


        else if (dataWindClean.hasTimedOut()) {
			// Timeout. Probably no joystick connected.

			rtx_message("Timeout while reading dataWindClean \n");}

		else if (dataBoat.hasTimedOut()) {
			// Timeout. Probably no joystick connected.

			rtx_message("Timeout while reading IMU-Data \n");}

		else
		{
			// Something strange happend. Critical Error.
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}
            rtx_timer_sleep(1);
	}

	return NULL;
}

// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}


int main (int argc, const char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Simulator Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DesiredHeading));



	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataWindClean, varname2, "WindCleanData"));
    DOB(store.registerVariable(dataBoat, varname4, "imuData"));
	//flags:
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
	DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));
	//destination of AVALON:
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
	DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));
	DOB(store.registerVariable(headingData, varname_course, "DesiredHeading"));


	// Start the working thread
    DOP(th = rtx_thread_create ("navi_simulator thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down Navi_Simulator...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
