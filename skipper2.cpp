/**
 * Skipper calls the navigations-programs and sets a current heading to the
 * store, so sailor can take over!!
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

#include "windcleaner.h"
#include "windsensor.h"
#include "imu.h"
#include "flags.h"
#include "waypoints.h"
#include "destination.h"
#include "desired_course.h"
//#include "weatherdata.h"

#define DEBUG_SKIPPER

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWindClean; //to get clean wind Data
DDXVariable waypointStruct;  //to initialize the waypointData 
DDXVariable dataBoat; //do get imu-Data
DDXVariable waypointData; //to store the calculated path
DDXVariable dataFlags;
DDXVariable dataNaviFlags;
DDXVariable dataRcFlags;
DDXVariable destinationStruct;
DDXVariable destinationData; //the actual values!
DDXVariable headingData; //the curr heading will be written here; sailor needs that!!!
//DDXVariable weatherData;

/**
 * Prototypes for utility functions
 * */
//int sign(int i);
//int sign(float i);


/**
 * Storage for the command line arguments
 * */


const char * varname_wypStruct = "wypStruct"; //does it have to be in the store as navidata, since its only the type of wypData?
const char * varname2 = "cleanwind";
const char * varname_wypData = "wypData";
const char * varname4 = "imu";
const char * varname_flags = "flags";
const char * varname_naviflags = "naviflags";
const char * varname_rcflags = "rcflags";
const char * varname_transf = "transfData";

const char * varname_destStruct = "destStruct";
const char * varname_destData = "destData";

const char * varname_course = "desiredheading";
//const char * varname_weather = "weather";


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
     {RTX_GETOPT_STR, &varname_destData, "Waypoints"},
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
#if 0
  {"weatherData", "Store Variable where the weather data is written",
   {
     {RTX_GETOPT_STR, &varname_weather, ""},
     RTX_GETOPT_END_ARG
   }
  },
#endif
  RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{

    WindCleanData cleanedwind;
    imuData boatData;
    Flags generalflags;
    NaviFlags naviflags;
    DestinationData destination; //actual array!!
    DesiredHeading desiredHeading; //course that goes to HE

    //for skipper purposes:
    
    // double distanceToWyp;
    double headingHistory[30], heading_average;
    double current_pos_longitude, current_pos_latitude; //already transformed and in meters

    int last_state;
    // int never_again = 0;
    // bool cplusplus_done = false;

    //for checking if destination-buoy is already passed!
    int current_buoy = 0;
    double vec_dist_buoy_x;
    double vec_dist_buoy_y;
    double vec_dist_nextbuoy_x, vec_dist_nextbuoy_y;
    // double dist_buoy;

    // new variables
    //
    int p;
    // int vec_buoy_curr_to_next_x, vec_buoy_curr_to_next_y;
    // double heading_buoy_curr_to_next;
    double heading_to_curr_buoy;
    double heading_to_next_buoy;
    // time_t start_time, end_time;
    // double buoytime;

    //fill the headingHistory:

    dataBoat.t_readto(boatData,0,0);
    for(int i=0; i<30; i++)
    {
        headingHistory[i] = boatData.attitude.yaw;
    }


	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (dataBoat.t_readto(boatData,10,1))
		{
            //waypointData.t_readto(waypoints,0,0);
            dataFlags.t_readto(generalflags,0,0);
            dataNaviFlags.t_readto(naviflags,0,0);
            destinationData.t_readto(destination,0,0);
            dataWindClean.t_readto(cleanedwind,0,0);
            headingData.t_readto(desiredHeading,0,0);
            //dataRcFlags.t_readto(rcflags,0,0);
            

            //fill the last heading into headingHistory and average it:
            headingHistory[29]= boatData.attitude.yaw;


            heading_average = 0;
            for (int u=0; u<30; u++)
            {
                heading_average += 1.0/30.0 * headingHistory[u];
            }

            if(!generalflags.autonom_navigation)
            {
                naviflags.navi_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                //dataNaviFlags.t_writefrom(naviflags);
                continue;
            }
            
            //bring the current_buoy counter up to speed
            for(p = 0; p < 10; p++)
            {
                current_buoy = p; 

                if(destination.Data[current_buoy].passed == 1) continue;
                break;
            }
            //calculate all the distances and vectors:


            current_pos_longitude =double (AV_EARTHRADIUS 
                *cos((boatData.position.latitude * AV_PI/180))*(AV_PI/180)
                *boatData.position.longitude);
            current_pos_latitude =double (AV_EARTHRADIUS
                *(AV_PI/180)*boatData.position.latitude);
            ///////
            vec_dist_buoy_x = destination.longitude - current_pos_longitude;
            vec_dist_buoy_y = destination.latitude - current_pos_latitude;
            //vec_dist_nextbuoy_x = destination.Data[current_buoy+1].longitude - current_pos_longitude;
            //vec_dist_nextbuoy_y = destination.Data[current_buoy+1].latitude - current_pos_latitude;
            ////////
            //new:
            //vec_buoy_curr_to_next_x = destination.Data[current_buoy +1].longitude - destination.Data[current_buoy].longitude;
            //vec_buoy_curr_to_next_y = destination.Data[current_buoy +1].latitude - destination.Data[current_buoy].latitude;
            //heading_buoy_curr_to_next = remainder((-atan2(vec_buoy_curr_to_next_y,vec_buoy_curr_to_next_x) + AV_PI/2),2*AV_PI); //richtiges heading, nicht mehr mathematisch! Radian!!
            heading_to_curr_buoy = remainder((-atan2(vec_dist_buoy_y,vec_dist_buoy_x) + AV_PI/2),2*AV_PI);//schon richtig gedreht!! noch in Radian!!! 
            heading_to_next_buoy = remainder((-atan2(vec_dist_nextbuoy_y,vec_dist_nextbuoy_x) + AV_PI/2),2*AV_PI);//schon richtig gedreht!! noch in Radian!!! 
            ////////
            //dist_buoy = sqrt((vec_dist_buoy_x*vec_dist_buoy_x) + (vec_dist_buoy_y*vec_dist_buoy_y));
            ///////

#ifdef DEBUG_SKIPPER
            rtx_message("dest.long %f, curr.long %f - dest.lat %f, curr.lat %f \n",destination.longitude,current_pos_longitude,destination.latitude,current_pos_latitude);
#endif


            //begin statemachine:
            switch(generalflags.navi_state)
            {
                case AV_FLAGS_NAVI_IDLE:
                    last_state = AV_FLAGS_NAVI_IDLE;

                    //continue; //don't do anything, start over the thread!!
                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_NAVI_NEWCALCULATION:
                   // if (fabs(desiredHeading.heading - heading_to_curr_buoy) > 1e-3)
                    {
                        //to write the heading into the store:
                        desiredHeading.heading = heading_to_curr_buoy*180/AV_PI;
                        //headingData.t_writefrom(desiredHeading);

                        //cplusplus_done = false;
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
#ifdef DEBUG_SKIPPER
                        rtx_message("new heading is %f, \n",desiredHeading.heading);
#endif
                    }
#if 0
                    if (dist_buoy < 12)
                    {
                        destinationData.t_readto(destination,0,0);
                        destination.Data[current_buoy].passed = 1;
                        destinationData.t_writefrom(destination);

#ifdef DEBUG_SKIPPER
                        rtx_message("boat heads to next buoy");
#endif
                    }
#endif
                    break;

            }


            // flags will be written to store:
            //headingData.t_writefrom(desiredHeading);

            //has to be modified:
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
        rtx_timer_sleep(0.1);
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
	rtx_main_init ("Skipper Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WaypointStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WaypointData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DesiredHeading));
    //DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Weather));



	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataWindClean, varname2, "WindCleanData"));
    DOB(store.registerVariable(dataBoat, varname4, "imuData"));
	//flags:
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
	DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));
	//navigation
	DOB(store.registerVariable(waypointStruct, varname_wypStruct, "WaypointStruct"));
	//DOB(store.registerVariable(waypointData, varname_wyp, "Waypoints"));
	//destination of AVALON:
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
	DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));
    //desired course for AVALON:
	DOB(store.registerVariable(headingData, varname_course, "DesiredHeading"));
    // Information about the wind
	//DOB(store.registerVariable(weatherData, varname_weather, "Weather"));


	// Start the working thread
    DOP(th = rtx_thread_create ("skipper thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down Skipper...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);

}
