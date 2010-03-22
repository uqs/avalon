/**
 * Thats a testing program to fill some artificial data into the store and call
 * the navigator!
 *
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

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
#include "imu.h"
#include "flags.h"
#include "destination.h"


/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWindClean; //to get clean wind Data
DDXVariable dataBoat; //do get imu-Data
DDXVariable dataNaviFlags;
DDXVariable destinationData;
DDXVariable destination; //the curr heading will be written here; sailor needs that!!!


/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(float i);


/**
 * Storage for the command line arguments
 * */


const char * varname_navi = "navidata"; //does it have to be in the store as navidata, since its only the type of wypData?
const char * varname2 = "cleanwind";
const char * varname_wyp = "wypData";
const char * varname4 = "imu";
const char * varname_naviflags = "naviflags";
const char * varname_transf = "transfData";
const char * varname_destData = "destData";
const char * varname_dest = "destination";


const char * producerHelpStr = "activating the navi-algorithm (3D-Planning)";

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
  {"navidata", "Variable where the calculated navi-waypoints are stored",
   {
     {RTX_GETOPT_STR, &varname3, "Waypoints"},
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

    WindCleanData cleanedWind;
    imuData boatData;
    NaviFlags naviflags;
    DestinationData destination;

    //intitialize the random seed:

    //srand ( time(NULL) );
    srand(123456);


	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (1) //dataBoat.t_readto(boatData,0,0))
		{
			//naviData.t_readto(boatData,0,0); //not necessary, isn't it?
            //dataFlags.t_readto(generalflags,0,0);
            //dataNaviFlags.t_readto(naviflags,0,0);
            //destinationData.t_readto(destinationCoord,0,0);
            //dataWindClean.t_readto(cleanedWind,0,0);

                       //setting destination and current position:

            destination.Data[0].longitude = double ( (rand()% 50000) / 1000.0); //generate fake gps data between 0 and 50 degrees
            destination.Data[0].latitude = double ( (rand()% 50000) / 1000.0);
            destination.Data[0].passed = 0;
            destination.Data[0].type = AV_DEST_TYPE_END;
            destination.Data[1].type = AV_DEST_TYPE_NOMORE;

            //rtx_message("The latitude shoud be: %f \n",destinationCoord.latitude);
            
            boatData.position.longitude = double ( (rand()% 500) / 1000.0) + destination.Data[0].longitude;
            boatData.position.latitude = double ( (rand()% 500) / 1000.0) + destination.Data[0].latitude;
            
            cleanedWind.speed_long = 10; //knots
            cleanedWind.global_direction_real_long = remainder((rand() % 360),360); //generate winddirection;


            //write the generated data into the store:
            dataBoat.t_writefrom(boatData);
            destinationData.t_writefrom(destination);
            dataWindClean.t_writefrom(cleanedWind);


            // set the trigger:

            
            naviflags.navi_state = 0;
            dataNaviFlags.t_writefrom(naviflags);

        }

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


// Some self-defined utility functions:
int sign(int i) // gives back the sign of an int
{
    if (i>=0)
        return 1;
    else
        return -1;
}

int sign(float i) // gives back the sign of a float
{
    if (i>=0)
        return 1;
    else
        return -1;
}



int main (int argc, char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Navigation interface Main", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
    //DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviData));
	//DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Waypoints));
	//DOC(DDX_STORE_REGISTER_TYPE (store.getId(), LakeTransformation));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));



	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataWindClean, varname2, "WindCleanData"));
    DOB(store.registerVariable(dataBoat, varname4, "imuData"));
	//flags:
	DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));
	
    //navigation
	//DOB(store.registerVariable(naviData, varname_navi, "NaviData"));
	//DOB(store.registerVariable(waypointData, varname_wyp, "Waypoints"));
	
    //tranformation details:
	//DOB(store.registerVariable(transformationData, varname_transf, "LakeTransformation"));
	
    //destination of AVALON:
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
	DOB(store.registerVariable(destination, varname_dest, "Destination"));


	// Start the working thread
    DOP(th = rtx_thread_create ("Wind cleaning thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down testaufruf...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
