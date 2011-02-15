/**
 * This program writes a new destination for the navigator *
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
#include "imu.h"
#include "ais.h"
#include "flags.h"
#include "destination.h"


//#define DEBUG_GLOBSKIPPER_EASY
//#define DEBUG_GLOBSKIPPER

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataBoat; //do get imu-Data
DDXVariable dataFlags;
DDXVariable dataRcFlags;
DDXVariable destinationStruct;
DDXVariable destinationData; //the actual values!
DDXVariable aisDestData;
DDXVariable skipperFlagData;
DDXVariable dataNaviFlags;


/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(double i);


/**
 * Storage for the command line arguments
 * */


const char * varname_navi = "navidata"; //does it have to be in the store as navidata, since its only the type of wypData?
const char * varname4 = "imu";
const char * varname_flags = "flags";
const char * varname_naviflags = "naviflags";
const char * varname_rcflags = "rcflags";

const char * varname_aisDestData = "aisDestData";
const char * varname_destStruct = "destStruct";
const char * varname_destData = "destData";

const char * varname_skipperflags = "skipperflags";

const char * producerHelpStr = "skipper help-string";

/**
 * Command line arguments   //has yet to be completed
 *
 * */
RtxGetopt producerOpts[] = {

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
    imuData boatData;
    Flags generalflags;
    DestinationData destination; //actual array!!
    SkipperFlags skipperflags;
    AisDestData ais_dest;

    double dest_dist = 1000;
    int i;
    unsigned int ais_dest_index_last=0;
    double distance_arr[1000];
    double distance_boat_dest_loc;
    double distance_boat_dest;
    double closest_distance;
    double last_global_wyp_longitude = 0.0;
    double last_global_wyp_latitude = 0.0;
    double vec_from_curr_to_last_wyp_x = 0.0; // in m
    double vec_from_curr_to_last_wyp_y = 0.0; // in m
    double angle_btw_boat_and_currwyp_and_lastwyp = 0.0; // in rad

    //initializing the call index
    skipperFlagData.t_readto(skipperflags,0,0);
    skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
    //     skipperflags.global_locator = AV_FLAGS_GLOBALSK_CLOSING;
    skipperFlagData.t_writefrom(skipperflags);

    aisDestData.t_readto(ais_dest,0,0);
    ais_dest_index_last=ais_dest.ais_dest_index;
    double current_pos_x, current_pos_y; //already transformed and in meters
    double dist_stored_dest_x, dist_stored_dest_y;



    while (1) {
        // Read the next data available, or wait at most 5 seconds
        if (dataBoat.t_readto(boatData,10,1))
        {
            dataFlags.t_readto(generalflags,0,0);
            destinationData.t_readto(destination,0,0);
            skipperFlagData.t_readto(skipperflags,0,0);
            aisDestData.t_readto(ais_dest,0,0);
            if(destination.destNr == 0)
            {
                skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                skipperFlagData.t_writefrom(skipperflags);
                // Put current position (the start position) as last waypoint,
                // because there is no real last waypoint...
                last_global_wyp_longitude = boatData.position.longitude;
                last_global_wyp_latitude = boatData.position.latitude;
            }

            // if we are on collision course, ais tells us the new destination point
            if (ais_dest.ais_dest_index != ais_dest_index_last)
            {
                rtx_message("listen to ais     index_last= %d index_ddx= %d\n", ais_dest_index_last, ais_dest.ais_dest_index);

                skipperflags.global_locator = ais_dest.global_skipper_flag;
                skipperFlagData.t_writefrom(skipperflags);
            }
            if ((generalflags.global_locator == ais_dest.global_skipper_flag) && (ais_dest.ais_dest_index != ais_dest_index_last))
            {
                rtx_message("aisflag has been written");
                // save the last waypoint in a temporary variable:
                last_global_wyp_longitude = destination.longitude;
                last_global_wyp_latitude = destination.latitude;

                destination.longitude = ais_dest.new_dest_long;
                destination.latitude = ais_dest.new_dest_lat;
                destination.skipper_index_call ++;
                destination.not_in_list = 1;
                destinationData.t_writefrom(destination);
                ais_dest_index_last=ais_dest.ais_dest_index;
            }

            // convert imu data to local 2D map
            current_pos_x =AV_EARTHRADIUS * cos((destination.latitude * AV_PI/180)) * (AV_PI/180)
                *(boatData.position.longitude-destination.longitude);
            current_pos_y =AV_EARTHRADIUS * (AV_PI/180) * (boatData.position.latitude-destination.latitude);

            // calculate vector from current to last global waypoint
            vec_from_curr_to_last_wyp_x =AV_EARTHRADIUS * cos((destination.latitude * AV_PI/180)) * (AV_PI/180)
                *(last_global_wyp_longitude - destination.longitude);
            vec_from_curr_to_last_wyp_y =AV_EARTHRADIUS * (AV_PI/180) * (last_global_wyp_latitude - destination.latitude);

            // distance to the next stored destination point
            dist_stored_dest_x =AV_EARTHRADIUS * cos((destination.latitude * AV_PI/180)) * (AV_PI/180)
                *(destination.Data[destination.destNr].longitude-destination.longitude);
            dist_stored_dest_y =AV_EARTHRADIUS * (AV_PI/180) * (destination.Data[destination.destNr].latitude-destination.latitude);

            // if we pass the current global waypoint without actually getting inside
            // the tolerance circle, we want to go to the next waypoint:

            distance_boat_dest = sqrt(pow(current_pos_x-dist_stored_dest_x,2) + pow(current_pos_y-dist_stored_dest_y,2)); 
            distance_boat_dest_loc = sqrt(pow(current_pos_x,2) + pow(current_pos_y,2)); //because the destination is always in (0,0)!

            // calculate angle between the vectors from boat to current waypoint and
            // from last waypoint to current waypoint
            angle_btw_boat_and_currwyp_and_lastwyp = acos((current_pos_x * vec_from_curr_to_last_wyp_x + current_pos_y * vec_from_curr_to_last_wyp_y)
                    /(distance_boat_dest_loc * sqrt(vec_from_curr_to_last_wyp_x * vec_from_curr_to_last_wyp_x + vec_from_curr_to_last_wyp_y * vec_from_curr_to_last_wyp_y)));
#ifdef DEBUG_GLOBSKIPPER_EASY
            rtx_message("Beginning: Notorious Angle: %f ",angle_btw_boat_and_currwyp_and_lastwyp*180.0/AV_PI);
#endif

            //begin statemachine: /////////////////////////////////////////////////
            switch(generalflags.global_locator)
            {
                case AV_FLAGS_GLOBALSK_LOCATOR:

                    //check which destination point we are closest
                    i = 0;
                    destination.destNr = 0;
                    while((i<AV_MAXNUM_DESTINATION) && (destination.Data[i].type != AV_DEST_TYPE_NOMORE ))
                    {

                        distance_arr[i] =   AV_EARTHRADIUS*AV_PI/180.0*sqrt(pow(boatData.position.latitude - destination.Data[i].latitude,2)
                                + pow(cos(destination.latitude*AV_PI/180.0)*(boatData.position.longitude - destination.Data[i].longitude),2));
#ifdef DEBUG_GLOBSKIPPER
                        rtx_message("Locator: destNr counter = %d, distance to boat = %f \n", i, distance_arr[i]);
#endif

                        if ( closest_distance > distance_arr[i] || i == 0)
                        {
                            closest_distance = distance_arr[i];

                            destination.destNr = i;

#ifdef DEBUG_GLOBSKIPPER
                            rtx_message("Locator: destNr definitiv = %d \n", destination.destNr);
#endif
                        } 

                        i++;
                    }
                    assert((destination.destNr < (AV_MAXNUM_DESTINATION+1)) && (destination.destNr>=0));

                    if (AV_EARTHRADIUS*AV_PI/180.0*sqrt(pow(destination.Data[destination.destNr].latitude - destination.Data[destination.destNr+1].latitude,2)
                                + pow(cos(destination.latitude*AV_PI/180.0)*(destination.Data[destination.destNr].longitude - destination.Data[destination.destNr+1].longitude),2))
                            > distance_arr[destination.destNr+1])
                    {
                        destination.destNr++;
                    }

                    // save the last waypoint in a temporary variable:
                    last_global_wyp_longitude = destination.longitude;
                    last_global_wyp_latitude = destination.latitude;
                    // write the current destination point to the store
                    destination.longitude = destination.Data[destination.destNr].longitude;
                    destination.latitude = destination.Data[destination.destNr].latitude;
                    destination.skipper_index_call ++;
                    destination.not_in_list = 0;
                    destinationData.t_writefrom(destination);
                    skipperflags.global_locator = AV_FLAGS_GLOBALSK_CLOSING;
                    skipperFlagData.t_writefrom(skipperflags);
#ifdef DEBUG_GLOBSKIPPER
                    rtx_message("Locator End: destData[]: %f / %f",destination.Data[destination.destNr].latitude,destination.Data[destination.destNr].longitude);
#endif

                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_GLOBALSK_CLOSING:

#ifdef DEBUG_GLOBSKIPPER_EASY
                    rtx_message("Closing: distance to destination = %f \n", distance_boat_dest_loc);
                    rtx_message("Closing: distance to stored destination = %f \n", distance_boat_dest);
#endif
                    // if the distance to the stored destination point is smaller then the previous defined distance
                    // then Avalon tries to reach the stored destination point
                    if (distance_boat_dest < dest_dist)
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_TRACKER;
                        skipperFlagData.t_writefrom(skipperflags);
                        // save the last waypoint in a temporary variable:
                        last_global_wyp_longitude = destination.longitude;
                        last_global_wyp_latitude = destination.latitude;

                        destination.longitude = destination.Data[destination.destNr].longitude;
                        destination.latitude = destination.Data[destination.destNr].latitude;
                        destinationData.t_writefrom(destination);
                        break;
                    }

                    // if the distance to the current destination point is to large or small, a new current destination point is calculated
                    if(distance_boat_dest_loc > 1.1*dest_dist || distance_boat_dest_loc < AV_GLOBALSKI_MIN_DIST_CURR_GLOB_WYP)
                    {
                        // save the last waypoint in a temporary variable:
                        last_global_wyp_longitude = destination.longitude;
                        last_global_wyp_latitude = destination.latitude;

                        destination.longitude = boatData.position.longitude + dest_dist/distance_boat_dest * (destination.Data[destination.destNr].longitude - boatData.position.longitude);
                        destination.latitude = boatData.position.latitude + dest_dist/distance_boat_dest * (destination.Data[destination.destNr].latitude - boatData.position.latitude);
                        destination.skipper_index_call ++;
                        destination.not_in_list = 1;
                        destinationData.t_writefrom(destination);

#ifdef DEBUG_GLOBSKIPPER
                        rtx_message("closing: generating new glob_dest, da zu weit weg!!! increased destination index to %d",destination.skipper_index_call);
#endif
                    }

                    /*	should not be necessary!
                        if(distance_boat_dest < 300)
                        {
                        destination.destNr += 1;
                        assert((destination.destNr < 1000) && (destination.destNr>=0));
                        destination.longitude = destination.Data[destination.destNr].longitude;
                        destination.latitude = destination.Data[destination.destNr].latitude;
                        destination.skipper_index_call ++;
                        destination.not_in_list = 0;
                        destinationData.t_writefrom(destination);

#ifdef DEBUG_GLOBSKIPPER
                        rtx_message("tracker: increased destination index to %d",destination.skipper_index_call);
#endif
                        }
                        */
                    break;
                    ////////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_GLOBALSK_TRACKER:
                    // in this case, Avalon tries to reach the original stored destination point
#ifdef DEBUG_GLOBSKIPPER_EASY
                    rtx_message("Tracker: distance to destination = %f, destNr = %d ",  distance_boat_dest_loc, destination.destNr);
#endif
                    if((distance_boat_dest < AV_GLOBALSKI_MIN_DIST_CURR_GLOB_WYP) && (destination.Data[destination.destNr].type != AV_DEST_TYPE_END)
                            || (angle_btw_boat_and_currwyp_and_lastwyp > AV_PI / 2.0))
                    {
                        destination.destNr += 1;
                        assert((destination.destNr < 1000) && (destination.destNr>=0));
                        // save the last waypoint in a temporary variable:
                        last_global_wyp_longitude = destination.longitude;
                        last_global_wyp_latitude = destination.latitude;

                        destination.longitude = destination.Data[destination.destNr].longitude;
                        destination.latitude = destination.Data[destination.destNr].latitude;
                        destination.skipper_index_call ++;
                        destination.not_in_list = 0;
                        destinationData.t_writefrom(destination);

                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_CLOSING;
                        skipperFlagData.t_writefrom(skipperflags);

#ifdef DEBUG_GLOBSKIPPER
                        rtx_message("tracker: increased destination index to %d",destination.skipper_index_call);
#endif
                    }   

                    if(distance_boat_dest_loc > 1.2*dest_dist)
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                        skipperFlagData.t_writefrom(skipperflags);
                    }

                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_GLOBALSK_AVOIDANCE:
                    // if aisEval computes a new destination point, we stay in this case
#ifdef DEBUG_GLOBSKIPPER
                    rtx_message("avoidance: distance to destination = %f \n", distance_boat_dest_loc);
#endif
                    if((distance_boat_dest_loc < 200) || (distance_boat_dest_loc > 2*dest_dist))
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                        skipperFlagData.t_writefrom(skipperflags);
                    }

                    break;
                    ////////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_GLOBALSK_SURVIVE:

#ifdef DEBUG_GLOBSKIPPER
                    rtx_message("survive: distance to destination = %f \n", distance_boat_dest);
#endif

                    if((distance_boat_dest_loc < 200) || (distance_boat_dest_loc > 600))
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                        skipperFlagData.t_writefrom(skipperflags);
                    }

                    break;
                    ////////////////////////////////////////////////////////////////////////////

            }

        }
#if 0
        else if (dataWindClean.hasTimedOut()) {
            // Timeout. Probably no joystick connected.

            rtx_message("Timeout while reading dataWindClean \n");}
#endif
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
    rtx_main_init ("Global Skipper Interface", RTX_ERROR_STDERR);


    // Open the store
    DOB(store.open());

    // Register the new Datatypes
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisDestData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), SkipperFlags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));


    //// Connect to variables, and create variables for the target-data
    DOB(store.registerVariable(dataBoat, varname4, "imuData"));
    //flags:
    DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
    //destination of AVALON:
    DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
    DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));
    DOB(store.registerVariable(aisDestData, varname_aisDestData, "AisDestData"));
    //desired course for AVALON:
    DOB(store.registerVariable(skipperFlagData, varname_skipperflags, "SkipperFlags"));
    DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));


    // Start the working thread
    DOP(th = rtx_thread_create ("GLOBAL skipper thread", 0,
                RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
                RTX_THREAD_CANCEL_DEFERRED,
                translation_thread, NULL,
                NULL, NULL));

    // Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
    rtx_message_routine ("Ctrl-C detected. Shutting down global Skipper...");

    // Terminating the thread
    rtx_thread_destroy_sync (th);

    // The destructors will take care of cleaning up the memory
    return (0);

}
