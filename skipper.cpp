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

// #define DEBUG_SKIPPER

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
int sign(int i);
int sign(double i);


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
    WaypointData waypoints;
    Flags generalflags;
    NaviFlags naviflags;
    DestinationData destination; //actual array!!
    DesiredHeading desiredHeading; //course that goes to HE

    //for skipper purposes:
    double heading_to_wyp;
    double heading_to_next_wyp;
    double dist_curr_wyp;
    double current_pos_x, current_pos_y; //already transformed and in meters
    //the four vectors to be needed are:
    double vec_dist_wyp_x,vec_dist_wyp_y;
    double vec_dist_wyp2_x,vec_dist_wyp2_y;
    double vec_fix_curr_x,vec_fix_curr_y;
    double vec_fix_next_x,vec_fix_next_y;
    double dist_solltrajectory, dist_next_trajectory;
    double dist_next_trajectory3, dist_next_trajectory2;

    //double area_soll, area_next;
    int current_wyp; // = 1;        //keeps track of which wyp we are navigating to!
    double entry_timestamp = 0.0;
    int last_state;
    int never_again = 0;

    //for checking if destination-buoy is already passed!
    int current_buoy = 0;
    double vec_dist_buoy_x;
    double vec_dist_buoy_y;
    double dist_buoy;
    unsigned long old_navi_index = 0;

    int p;
    double vec_prev_to_next_wyp_x, vec_prev_to_next_wyp_y;
    double heading_curr_to_next_wyp;
    double heading_prev_to_next_wyp;

    dataBoat.t_readto(boatData,0,0);
    std::vector<double> headingHistory(30,boatData.attitude.yaw);
    double heading_average;
    dataWindClean.t_readto(cleanedwind,0,0);
    //std::vector<double> dir_wind_hist(30,cleanedwind.global_direction_real_long);
    double dir_wind_mean;

    //initializing the call index
    dataNaviFlags.t_readto(naviflags,0,0);
    naviflags.navi_index_call = 0;
    dataNaviFlags.t_writefrom(naviflags);


    while (1) {
        // Read the next data available, or wait at most 5 seconds
        if (dataBoat.t_readto(boatData,10,1))
        {
            waypointData.t_readto(waypoints,0,0);
            dataFlags.t_readto(generalflags,0,0);
            dataNaviFlags.t_readto(naviflags,0,0);
            destinationData.t_readto(destination,0,0);
            dataWindClean.t_readto(cleanedwind,0,0);
            headingData.t_readto(desiredHeading,0,0);

	    // compute the mean of the heading and the winddirection
	    heading_average = 0;
	    headingHistory.insert(headingHistory.begin(),boatData.attitude.yaw);
	    headingHistory.resize(30);
	    for (unsigned int u=0; u<headingHistory.size(); u++)
            {
                heading_average += 1.0/headingHistory.size() * headingHistory[u];
            }
	    
// 	    dir_wind_mean = dir_wind_hist[0];
// 	    dir_wind_hist.insert(dir_wind_hist.begin(),cleanedwind.global_direction_real_long);
// 	    dir_wind_hist.resize(30);
// 	    for (unsigned int u=1; u<dir_wind_hist.size(); u++)
//             {
//                 dir_wind_mean += 1.0/dir_wind_hist.size() * remainder(dir_wind_hist[u]-dir_wind_hist[0],360.0);
//             }
// 	    dir_wind_mean=remainder(dir_wind_mean,360.0);
	dir_wind_mean = cleanedwind.global_direction_real;
// rtx_message("dir_wind_mean: %f",dir_wind_mean);
            if(!generalflags.autonom_navigation)
            {
                naviflags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION;
                naviflags.navi_index_call = old_navi_index + 1;
                dataNaviFlags.t_writefrom(naviflags);

                rtx_timer_sleep(0.3);
                continue;
            }

            //bring the current_buoy counter up to speed
            for(p = 0; p < 100; p++)
            {
                current_wyp = p; 

                if(waypoints.Data[current_wyp].passed == 1) 
                {
                    continue;
                }
                break;
            }
	    
#ifdef DEBUG_SKIPPER
	    rtx_message("next WP: x= %lf, y= %lf head= %f",waypoints.Data[current_wyp].x,
			    waypoints.Data[current_wyp].y, waypoints.Data[current_wyp].heading);
#endif

            //write the current desired heading to store:
            desiredHeading.heading = waypoints.Data[current_wyp].heading;
            headingData.t_writefrom(desiredHeading);
            
            
            dist_next_trajectory3 = dist_next_trajectory2;
            dist_next_trajectory2 = dist_next_trajectory;
            
            //calculate all the distances and vectors:
            current_pos_x =double (AV_EARTHRADIUS * (AV_PI/180)
                *(boatData.position.latitude - destination.latitude));
            current_pos_y =double (AV_EARTHRADIUS 
                *cos((destination.latitude * AV_PI/180))*(AV_PI/180)
                *(boatData.position.longitude - destination.longitude));


            ///////
            vec_dist_wyp_x = waypoints.Data[current_wyp].x - current_pos_x; //everything already in meters
            vec_dist_wyp_y = waypoints.Data[current_wyp].y - current_pos_y; //everything already in meters
            ///////
            vec_dist_wyp2_x = waypoints.Data[current_wyp+1].x - current_pos_x; //everything already in meters
            vec_dist_wyp2_y = waypoints.Data[current_wyp+1].y - current_pos_y; //everything already in meters
            ///////
            vec_fix_next_x = waypoints.Data[current_wyp+1].x - waypoints.Data[current_wyp].x;
            vec_fix_next_y = waypoints.Data[current_wyp+1].y - waypoints.Data[current_wyp].y;
            ///////
            vec_fix_curr_x = waypoints.Data[current_wyp].x - waypoints.Data[current_wyp-1].x;
            vec_fix_curr_y = waypoints.Data[current_wyp].y - waypoints.Data[current_wyp-1].y;
            ///////
	    vec_dist_buoy_x = AV_EARTHRADIUS*AV_PI/180*(destination.Data[current_buoy].latitude - destination.latitude) - current_pos_x;
            vec_dist_buoy_y = AV_EARTHRADIUS*cos(destination.latitude * AV_PI/180)*AV_PI/180
			      *(destination.Data[current_buoy].longitude - destination.longitude) - current_pos_y;
            ////////
            vec_prev_to_next_wyp_x =waypoints.Data[current_wyp+1].x - waypoints.Data[current_wyp-1].x;
            vec_prev_to_next_wyp_y =waypoints.Data[current_wyp+1].y - waypoints.Data[current_wyp-1].y;
            ///////
            dist_curr_wyp = sqrt((vec_dist_wyp_x * vec_dist_wyp_x) + (vec_dist_wyp_y * vec_dist_wyp_y));
            heading_to_wyp = remainder((-atan2(vec_dist_wyp_y,vec_dist_wyp_x) + AV_PI/2),2*AV_PI); //schon richtig genullt, nicht mehr mathematisch!
//             heading_to_next_wyp = remainder((-(atan2(vec_dist_wyp2_y,vec_dist_wyp2_x)) + AV_PI/2),2*AV_PI); //richtig genullt!! 
//             heading_prev_to_next_wyp = remainder((-(atan2(vec_prev_to_next_wyp_y,vec_prev_to_next_wyp_x)) + AV_PI/2),2*AV_PI); //richtig genullt!! 
// 	    heading_curr_to_next_wyp = remainder((-(atan2(vec_fix_next_y,vec_fix_next_x)) + AV_PI/2),2*AV_PI); //richtig genullt!! 
	    heading_to_next_wyp = remainder((atan2(vec_dist_wyp2_y,vec_dist_wyp2_x)),2*AV_PI); //richtig genullt!! 
	    heading_prev_to_next_wyp = remainder((atan2(vec_prev_to_next_wyp_y,vec_prev_to_next_wyp_x)),2*AV_PI); //richtig genullt!!
            heading_curr_to_next_wyp = remainder((atan2(vec_fix_next_y,vec_fix_next_x)),2*AV_PI); //richtig genullt!! 
// rtx_message("head_prev_next = %f, head_pose_next = %f, head_curr_next = %f ", heading_prev_to_next_wyp*180.0/AV_PI, heading_to_next_wyp*180.0/AV_PI, heading_curr_to_next_wyp*180.0/AV_PI);
            ///////
            dist_solltrajectory = (vec_dist_wyp_x * vec_fix_curr_y - vec_dist_wyp_y * vec_fix_curr_x) / 
		    (sqrt((double) (vec_fix_curr_x * vec_fix_curr_x) + (double) (vec_fix_curr_y * vec_fix_curr_y)));
            dist_next_trajectory = fabs((vec_dist_wyp_x * vec_fix_next_y - vec_dist_wyp_y * vec_fix_next_x) / 
			    (sqrt((double) (vec_fix_next_x * vec_fix_next_x) + (double) (vec_fix_next_y * vec_fix_next_y))));
            dist_buoy = sqrt((vec_dist_buoy_x*vec_dist_buoy_x) + (vec_dist_buoy_y*vec_dist_buoy_y));
            ///////

#ifdef DEBUG_SKIPPER
	    rtx_message("dist to next trajectory: %f \n", dist_next_trajectory);
            rtx_message("current_wyp = %d, desired heading = %f \n",current_wyp,desiredHeading.heading);
#endif

            //begin statemachine:

            switch(generalflags.navi_state)
            {
                case AV_FLAGS_NAVI_IDLE:
                    last_state = AV_FLAGS_NAVI_IDLE;
                    //don't do anything, start over the thread!!
                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_NAVI_NEWCALCULATION:

                    never_again = 0;

                    ///get back to normalnavigation:
                    if(generalflags.navi_index_answer == generalflags.navi_index_call)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("newcalc: switching to normalnavigation\n");
#endif
                        //switch to state normalsailing!!
                        naviflags.navi_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        dataNaviFlags.t_writefrom(naviflags);
                        entry_timestamp = waypointData.timeStamp();
                        old_navi_index = generalflags.navi_index_call;
                        //current_wyp = 1;
                    }



                    if(waypoints.Data[current_wyp].wyp_type != 0)
                    {
                   
#ifdef DEBUG_SKIPPER
                        rtx_message("newcalc: dist to next trajectory: %f meters\n", dist_next_trajectory);
                        rtx_message("newcalc: dist to curr wyp: %f meters\n", dist_curr_wyp);
#endif

                        //go through all the conditions and take measures:
                        //
#if 0
                        //GO INTO GOAL APPROACH
                        if(waypoints.Data[current_wyp].wyp_type == AV_WYP_TYPE_END)
                        {
#ifdef DEBUG_SKIPPER
                            rtx_message("going into goal approach mode");
#endif
                            naviflags.navi_state = AV_FLAGS_NAVI_GOAL_APPROACH;
                            dataNaviFlags.t_writefrom(naviflags);
                            last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        }
#endif
                        //HEAD TO THE NEXT WAYPOINT
                        if(((dist_next_trajectory < 20.0) || (dist_curr_wyp < 20.0)) 
                                 && waypoints.Data[current_wyp].wyp_type != AV_WYP_TYPE_END)
                        {
                            waypoints.Data[current_wyp].passed = 1;
                            waypointData.t_writefrom(waypoints);
#ifdef DEBUG_SKIPPER
                            rtx_message("newcalc: nächster wyp ansteuern (new wyp = %d)", current_wyp+1);
#endif
                        }
                    }
                    break;
                    /////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_NAVI_NORMALNAVIGATION:

#ifdef DEBUG_SKIPPER
                    rtx_message("normalnavi: dist to next trajectory: %f meters\n", dist_next_trajectory);
                    rtx_message("normalnavi: dist to curr wyp: %f meters\n", dist_curr_wyp);
		    rtx_message("current_wyp = %d, dist = %f  dx = %f dy = %f \n",current_wyp,dist_curr_wyp, 
				    waypoints.Data[current_wyp].x - current_pos_x, 
				    waypoints.Data[current_wyp].y - current_pos_y);
#endif
                    //go through all the conditions and take measures:
                    //
                    //GO INTO GOAL APPROACH
                    if(waypoints.Data[current_wyp].wyp_type == AV_WYP_TYPE_END)
                    {
#ifdef DEBUG_SKIPPER
                        rtx_message("normalnavi: going into goal approach mode\n");
#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_GOAL_APPROACH;
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                    }

                    //HEAD TO THE NEXT WAYPOINT
                    if(((dist_next_trajectory < 40.0) || (dist_curr_wyp < 40.0) 
                                /*|| ((sign((remainder(heading_curr_to_next_wyp - heading_to_next_wyp,2*AV_PI)))
                                        *sign(remainder(heading_curr_to_next_wyp - heading_prev_to_next_wyp,2*AV_PI))) == -1)*/)
                            && waypoints.Data[current_wyp].wyp_type != AV_WYP_TYPE_END)
                    {
                        waypoints.Data[current_wyp].passed = 1;
                        waypointData.t_writefrom(waypoints);

//#ifdef DEBUG_SKIPPER
                        rtx_message("normalnavi: nächster wyp ansteuern (new wyp = %d)\n", current_wyp+1);
//#endif
                        last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                    }

#ifdef DEBUG_SKIPPER
                    rtx_message("heading_curr_to_next: %f; heading_to_next: %f; course_prev_to_next: %f; \n",heading_curr_to_next_wyp*180/AV_PI,
				    heading_to_next_wyp*180/AV_PI,heading_prev_to_next_wyp*180/AV_PI);
#endif
                    // DO A NEWCALCULATION -> GO INTO NEWCALC MODE
                    if(((fabs(remainder(dir_wind_mean - waypoints.Data[current_wyp].winddirection,360.0)) > 10.0)
			    /*|| ((dist_next_trajectory > dist_next_trajectory2) && (dist_next_trajectory2 > dist_next_trajectory3) 
			    && (dist_next_trajectory > 100.0))*/ || (fabs(dist_solltrajectory) > 100.0))
			    /*|| ((waypoints.Data[current_wyp].wyp_type == AV_WYP_TYPE_END) && (dist_curr_wyp < 80.0))*/
			    || (((remainder(heading_prev_to_next_wyp - heading_to_next_wyp-0.1,2*AV_PI)>0 && (remainder(heading_curr_to_next_wyp-heading_to_next_wyp-0.04,2*AV_PI)>0))
			    || (remainder(heading_prev_to_next_wyp-heading_to_next_wyp+0.1,2*AV_PI)<0 && (remainder(heading_curr_to_next_wyp-heading_to_next_wyp+0.04,2*AV_PI)<0)))
				&& fabs(dist_curr_wyp) > 400.0 && (waypoints.Data[current_wyp].wyp_type != AV_WYP_TYPE_END)))
                        {
// #ifdef DEBUG_SKIPPER
                            rtx_message("normalnavi: switching to newcalculation state; reason:");
			    if(fabs(dir_wind_mean - waypoints.Data[current_wyp].winddirection) > 10.0)
                            {
                                rtx_message("wind has changed");
                            }
 /*
			    if ((waypoints.Data[current_wyp].wyp_type == AV_WYP_TYPE_END) && (dist_curr_wyp < 80.0))
			    {
				rtx_message("reached last waypoint");
			    }*/
                            if (fabs(dist_solltrajectory) > 100.0)
                            {
                                rtx_message("dist_solltrajectory too bigi (%f meters) ",dist_solltrajectory);
                            }

// #endif
                            naviflags.navi_state = AV_FLAGS_NAVI_NEWCALCULATION;
                            naviflags.navi_index_call ++;
                            dataNaviFlags.t_writefrom(naviflags);
                            last_state = AV_FLAGS_NAVI_NORMALNAVIGATION;
                        }

                    never_again = 0;

                    break;
                    ///////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_NAVI_GOAL_APPROACH:
                    //if AVALON approaches the final destination, give it
                    //always the direct heading:!!!!!!
                    if (dist_curr_wyp < 50.0)
                    {
//#ifdef DEBUG_SKIPPER
                        rtx_message("You are there, congratulations");
//#endif
                        naviflags.navi_state = AV_FLAGS_NAVI_IDLE;
                        dataNaviFlags.t_writefrom(naviflags);
                    }
                    desiredHeading.heading = heading_to_wyp*180/AV_PI;
                    headingData.t_writefrom(desiredHeading);
                    last_state = AV_FLAGS_NAVI_GOAL_APPROACH;
                    break;
            }
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

// Some self-defined utility functions:
int sign(int i) // gives back the sign of an int
{
	if (i>=0)
		return 1;
	else
		return -1;
}

int sign(double i) // gives back the sign of a float
{
	if (i>=0)
		return 1;
	else
		return -1;
}


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
	DOB(store.registerVariable(waypointData, varname_wypData, "WaypointData"));
	//tranformation details:
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
