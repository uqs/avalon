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
#include "coord_transform.h"
#include "destination.h"
#include "desired_course.h"
//#include "weatherdata.h"

#define DEBUG_SKIPPER

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWindClean; //to get clean wind Data
DDXVariable naviData;  //to initialize the waypointData 
DDXVariable dataBoat; //do get imu-Data
DDXVariable waypointData; //to store the calculated path
DDXVariable dataFlags;
DDXVariable dataNaviFlags;
DDXVariable dataRcFlags;
DDXVariable transformationData; 
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


const char * varname_navi = "navidata"; //does it have to be in the store as navidata, since its only the type of wypData?
const char * varname2 = "cleanwind";
const char * varname_wyp = "wypData";
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
    Waypoints waypoints;
    Flags generalflags;
    NaviFlags naviflags;
	//rcFlags rcflags;
    LakeTransformation transformation;
    //Destination destinationCoord; 
    DestinationData destination; //actual array!!
    DesiredHeading desiredHeading; //course that goes to HE

    //for skipper purposes:
    
    //double degreeDeviation;
    //double courseDeviation;
    double heading_to_wyp;
    double heading_to_next_wyp;
    double distanceToWyp;
    double headingHistory[30], heading_average;
    int current_pos_longitude, current_pos_latitude; //already transformed and in meters
    //the four vectors to be needed are:
    int vec_dist_wyp_x,vec_dist_wyp_y;
    int vec_dist_wyp2_x,vec_dist_wyp2_y;
    int vec_fix_curr_x,vec_fix_curr_y;
    int vec_fix_next_x,vec_fix_next_y;
    double dist_solltrajectory, dist_next;

    //double area_soll, area_next;
    int current_wyp = 1;        //keeps track of which wyp we are navigating to!
    double last_timestamp = 0.0;
    int last_state;
    int never_again = 0;
    bool cplusplus_done = false;

    //for checking if destination-buoy is already passed!
    int current_next_buoy = 0;
    int vec_dist_buoy_x;
    int vec_dist_buoy_y;
    double dist_buoy;

    int counter = 1;
    int first_longitude;
    int first_latitude;
    int meters;

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
            waypointData.t_readto(waypoints,0,0);
            dataFlags.t_readto(generalflags,0,0);
            //dataNaviFlags.t_readto(naviflags,0,0);
            transformationData.t_readto(transformation,0,0);
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
#if 0
            if(!generalflags.autonom_navigation)
            {
                naviflags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION;
                naviflags.navi_calculation = AV_FLAGS_NAVI_ENABLER_OFF;
                dataNaviFlags.t_writefrom(naviflags);
                continue;
            }
#endif
            //calculate all the distances and vectors:


            current_pos_longitude = AV_EARTHRADIUS 
                *cos((boatData.position.latitude * AV_PI/180))*(AV_PI/180)
                *boatData.position.longitude;
            current_pos_latitude = AV_EARTHRADIUS
                *(AV_PI/180)*boatData.position.latitude;

            if (counter == 1)
            {
                first_longitude = AV_EARTHRADIUS 
                    *cos((boatData.position.latitude * AV_PI/180))*(AV_PI/180)
                    *boatData.position.longitude;
                first_latitude = AV_EARTHRADIUS
                    *(AV_PI/180)*boatData.position.latitude;
                counter++;
            }
            meters = (int) sqrt(double((current_pos_longitude - first_longitude)*(current_pos_longitude - first_longitude) + 
                        (current_pos_latitude - first_latitude)*(current_pos_latitude - first_latitude)));
             
            FILE * testfile2;
            testfile2 = fopen("gps_googletest.kml","a");

            fprintf(testfile2," <coordinates>%f,%f,0</coordinates>\n",boatData.position.longitude,boatData.position.latitude);
            fclose(testfile2);



            FILE * testfile;
            testfile = fopen("gps_testlogs","a");

            fprintf(testfile,"%d %d %d \n",current_pos_longitude, current_pos_latitude, meters);

            fclose(testfile);

            /////////////////////////////////////////////////////////////
            ///////
#if 0
            vec_dist_wyp_x = waypoints.Data[current_wyp].longitude - current_pos_longitude; //everything already in meters
            vec_dist_wyp_y = waypoints.Data[current_wyp].latitude - current_pos_latitude; //everything already in meters
            ///////
            vec_dist_wyp2_x = waypoints.Data[current_wyp+1].longitude - current_pos_longitude; //everything already in meters
            vec_dist_wyp2_y = waypoints.Data[current_wyp+1].latitude - current_pos_latitude; //everything already in meters
            ///////
            vec_fix_next_x = waypoints.Data[current_wyp+1].longitude - waypoints.Data[current_wyp].longitude;
            vec_fix_next_y = waypoints.Data[current_wyp+1].latitude - waypoints.Data[current_wyp].latitude;
            ///////
            vec_fix_curr_x = waypoints.Data[current_wyp].longitude - waypoints.Data[current_wyp-1].longitude;
            vec_fix_curr_y = waypoints.Data[current_wyp].latitude - waypoints.Data[current_wyp-1].latitude;
            ///////
            vec_dist_buoy_x = destination.Data[current_next_buoy].longitude - current_pos_longitude;
            vec_dist_buoy_y = destination.Data[current_next_buoy].latitude - current_pos_latitude;
            ////////
            distanceToWyp = sqrt((vec_dist_wyp_x * vec_dist_wyp_x) + (vec_dist_wyp_y * vec_dist_wyp_y));
            heading_to_wyp = remainder((-atan2(vec_dist_wyp_y,vec_dist_wyp_x) + AV_PI/2),2*AV_PI); //schon richtig genullt, nicht mehr mathematisch!
            heading_to_next_wyp = remainder((-(atan2(vec_dist_wyp2_y,vec_dist_wyp2_x))+AV_PI/2),2*AV_PI); //richtig genullt!! 
            ///////
            dist_solltrajectory = (vec_dist_wyp_x * vec_fix_curr_y - vec_dist_wyp_y * vec_fix_curr_x) / (sqrt((double) vec_fix_curr_x * vec_fix_curr_x + (double) vec_fix_curr_y * vec_fix_curr_y));
            dist_next = (vec_dist_wyp_x * vec_fix_next_y - vec_dist_wyp_y * vec_fix_next_x) / (sqrt((double) vec_fix_next_x * vec_fix_next_x + (double) vec_fix_next_y * vec_fix_next_y));
            dist_buoy = sqrt((vec_dist_buoy_x*vec_dist_buoy_x) + (vec_dist_buoy_y*vec_dist_buoy_y));
            ///////

            //write what buoys have been passed
            if(destination.Data[current_next_buoy].type != AV_DEST_TYPE_END)
            {
                if(dist_buoy < AV_NAVI_GRID_SIZE)
                {
                    destination.Data[current_next_buoy].passed = 1;
                    current_next_buoy++;
                    destinationData.t_writefrom(destination);
                }
            }


            //begin statemachine:
            switch(generalflags.navi_state)
            {
                case AV_FLAGS_NAVI_IDLE:
                    last_state = AV_FLAGS_NAVI_IDLE;
                    rtx_timer_sleep(0.1);
                    continue; //don't do anything, start over the thread!!
                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_NAVI_NEWCALCULATION:
                    if(last_state != AV_FLAGS_NAVI_NEWCALCULATION)
                    {
                        last_timestamp = waypointData.timeStamp();
                        last_state = AV_FLAGS_NAVI_NEWCALCULATION;
                        naviflags.navi_calculation = AV_FLAGS_NAVI_ENABLER_ON;
                        dataNaviFlags.t_writefrom(naviflags);
                    }
                    else if(last_timestamp != waypointData.timeStamp())
                    {
                        //switch to state normalsailing!!
                        naviflags.navi_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        naviflags.navi_calculation = AV_FLAGS_NAVI_ENABLER_OFF;
                        dataNaviFlags.t_writefrom(naviflags);
                        current_wyp = 1;
                    }
                    never_again = 0;
                    break;
                    /////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_NAVI_NORMALNAVIGATION:
                    //if (fabs(desiredHeading.heading - waypoints.Data[current_wyp].heading) > 1e-5)
                    //{
                        //to write the heading into the store:
                        desiredHeading.heading = waypoints.Data[current_wyp].heading;
                        headingData.t_writefrom(desiredHeading);
                        cplusplus_done = false;
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
#ifdef DEBUG_SKIPPER
                        rtx_message("neues heading ist: %f \n",desiredHeading.heading);
#endif
                    //}

                    //go through all the conditions and take measures:
                    if(waypoints.Data[current_wyp].wyp_type == AV_WYP_TYPE_END)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("end wyp reached");
#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_GOAL_APPROACH;
                        dataNaviFlags.t_writefrom(naviflags);
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        continue;
                    }
                    if(waypoints.Data[current_wyp].wyp_type == AV_WYP_TYPE_BUOY)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("buoy type reached");
#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_BUOY_APPROACH;
                        naviflags.buoy_maneuver = 1;
                        dataNaviFlags.t_writefrom(naviflags);
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        continue;
                    }
#if 0
                    if((dist_next < AV_NAVI_LAKE_DIST_FOR_MANEUVER) && !cplusplus_done)
                    {
                        current_wyp++;
                        cplusplus_done = true;
#ifdef DEBUG_SKIPPER
                        rtx_message("nächster wyp ansteuern");
#endif
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        continue;
                    }
#endif
                    //if(fabs(remainder((heading_to_wyp - desiredHeading.heading*AV_PI/180),2*AV_PI)) > (45*AV_PI/180))
                    if(distanceToWyp < AV_NAVI_GRID_SIZE && !cplusplus_done)
                    { 
                        current_wyp++;
                        cplusplus_done = true;
#ifdef DEBUG_SKIPPER
                        rtx_message("nächster wyp ansteurn(2)");
                        rtx_message("desiredheading = %f, heading to wyp = %f, resultat = %f \n",desiredHeading.heading,heading_to_wyp*180/AV_PI,(heading_to_wyp - desiredHeading.heading*AV_PI/180)*180/AV_PI);
#endif
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        continue;
                    }
#if 0
                    if(dist_solltrajectory > 50.0)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("dist solltrajectory zu gross -> newcalc");
#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION;
                        dataNaviFlags.t_writefrom(naviflags);
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        continue;
                    }
#endif
                    //to check if wind is still more or less the same
                    //TODO: test if 20 degrees is ok or to much!!!
                    if(fabs(cleanedwind.global_direction_real_long - waypoints.Data[current_wyp].winddirection) >40.0)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("wind has changed");
#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION;
                        dataNaviFlags.t_writefrom(naviflags);
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        continue;
                    }
                    never_again = 0;

                    break;
                    ///////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_NAVI_GOAL_APPROACH:
                    //if AVALON approaches the final destination, give it
                    //always the direct heading:!!!!!!
                    if (distanceToWyp < 5.0)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("You are there, congratulations");
#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_IDLE;
                        dataNaviFlags.t_writefrom(naviflags);
                    }
                    desiredHeading.heading = heading_to_wyp*180/AV_PI;
                    headingData.t_writefrom(desiredHeading);
                    last_state = AV_FLAGS_NAVI_GOAL_APPROACH;
                    break;
                    ///////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_NAVI_BUOY_APPROACH:
                    //calculate the heading to go around the buoy!!
                    desiredHeading.heading = (heading_to_wyp + AV_NAVI_BUOY_MAN_HEADINGDEV)*180/AV_PI;
                    headingData.t_writefrom(desiredHeading);

                    //get back into the normal sailing state:
                    if ((fabs(heading_to_wyp - heading_to_next_wyp) < 10*AV_PI/180) && !never_again)
                    {
                        current_wyp++;
                        naviflags.navi_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        dataNaviFlags.t_writefrom(naviflags);
                        never_again = 1;
                    }
                    last_state = AV_FLAGS_NAVI_BUOY_APPROACH;
                    break;

            }

            
            // flags will be written to store:
            //headingData.t_writefrom(desiredHeading);

            //has to be modified:
#endif
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


int main (int argc, char * argv[])
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
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Waypoints));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), LakeTransformation));
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
	DOB(store.registerVariable(naviData, varname_navi, "NaviData"));
	DOB(store.registerVariable(waypointData, varname_wyp, "Waypoints"));
	//tranformation details:
	DOB(store.registerVariable(transformationData, varname_transf, "LakeTransformation"));
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
