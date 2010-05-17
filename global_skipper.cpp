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


// #define DEBUG_GLOBSKIPPER

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
    NaviFlags naviflags;
    AisDestData ais_dest;

    double dest_dist = 2000;
    double closest_distance;
    int i,p;
    bool first=true;
    unsigned int ais_dest_index_last=0;
    double distance;

    //initializing the call index
    skipperFlagData.t_readto(skipperflags,0,0);
    skipperflags.skip_index_dest_call = 0;
    skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
    skipperFlagData.t_writefrom(skipperflags);
            

    double current_pos_longitude, current_pos_latitude; //already transformed and in meters


    while (1) {
        // Read the next data available, or wait at most 5 seconds
        if (dataBoat.t_readto(boatData,10,1))
        {
            dataFlags.t_readto(generalflags,0,0);
            destinationData.t_readto(destination,0,0);
            skipperFlagData.t_readto(skipperflags,0,0);
	    aisDestData.t_readto(ais_dest,0,0);
            //calculate all the distances and vectors:
// rtx_message("index: %d      last: %d",ais_dest.ais_dest_index,ais_dest_index_last);
if (ais_dest.ais_dest_index != ais_dest_index_last)
  {
rtx_message("listen to ais     index_last= %d\n", ais_dest_index_last);
destination.longitude = ais_dest.new_dest_long;
destination.latitude = ais_dest.new_dest_lat;
destinationData.t_writefrom(destination);
skipperflags.global_locator = ais_dest.global_skipper_flag;
skipperflags.skip_index_dest_call ++;
skipperFlagData.t_writefrom(skipperflags);
// while(generalflags.global_locator != ais_dest.global_skipper_flag)
// {
// skipperflags.global_locator = ais_dest.global_skipper_flag;
// skipperFlagData.t_writefrom(skipperflags);
// }
      
  }
if (generalflags.global_locator == ais_dest.global_skipper_flag)
{
ais_dest_index_last=ais_dest.ais_dest_index;
}
            current_pos_longitude =double (AV_EARTHRADIUS 
                    *cos((boatData.position.latitude * AV_PI/180))*(AV_PI/180)
                    *boatData.position.longitude);
            current_pos_latitude =double (AV_EARTHRADIUS
                    *(AV_PI/180)*boatData.position.latitude);

#ifdef DEBUG_GLOBSKIPPER
            rtx_message("the current global destpoint is number %d flags.global_locator = %d \n", destination.destNr, generalflags.global_locator);
#endif
// rtx_message("the current global destpoint is number %d flags.global_locator = %d \n", destination.destNr, generalflags.global_locator);
            //begin statemachine: /////////////////////////////////////////////////
            switch(generalflags.global_locator)
            {
                case AV_FLAGS_GLOBALSK_LOCATOR:

                    //check what wyp we are closest    
                    closest_distance = sqrt((current_pos_longitude - destination.Data[0].longitude)
                            *(current_pos_longitude - destination.Data[0].longitude)
                            + (current_pos_latitude - destination.Data[0].latitude)
                            *(current_pos_latitude - destination.Data[0].latitude));
                    i = 0;
                    destination.destNr = 0;

#ifdef DEBUG_GLOBSKIPPER
                    rtx_message("locator: closest distance = %f \n", closest_distance);
#endif
                    while((i<1000) && (destination.Data[i].type != AV_DEST_TYPE_NOMORE ))
                    {

#ifdef DEBUG_GLOBSKIPPER
                        rtx_message("destNr zähler = %d, distanz zum bood = %f \n", i, (sqrt((current_pos_longitude - destination.Data[i].longitude)
                                        *(current_pos_longitude - destination.Data[i].longitude)
                                        + (current_pos_latitude - destination.Data[i].latitude)
                                        *(current_pos_latitude - destination.Data[i].latitude))));
#endif

                        if ( closest_distance > sqrt((current_pos_longitude - destination.Data[i].longitude)
                                    *(current_pos_longitude - destination.Data[i].longitude)
                                    + (current_pos_latitude - destination.Data[i].latitude)
                                    *(current_pos_latitude - destination.Data[i].latitude)))
                        {
                            closest_distance = sqrt((current_pos_longitude - destination.Data[i].longitude)
                                    *(current_pos_longitude - destination.Data[i].longitude)
                                    + (current_pos_latitude - destination.Data[i].latitude)
                                    *(current_pos_latitude - destination.Data[i].latitude));

                            destination.destNr = i;

#ifdef DEBUG_GLOBSKIPPER
                            rtx_message("destNr definitiv = %d \n", destination.destNr);
#endif
                        } 

                        i++;
                    }

                    assert((destination.destNr < 1000) && (destination.destNr>=0));
                    //assert((i < 1000) && (i>=0));

                    for (p = 0; p < 3; p++)
                    {
                        rtx_message ("locator: counter p = %d \n",p);
                        if (((sqrt((current_pos_longitude - destination.Data[destination.destNr+2-p].longitude)
                                            *(current_pos_longitude - destination.Data[destination.destNr+2-p].longitude)
                                            + (current_pos_latitude - destination.Data[destination.destNr+2-p].latitude)
                                            *(current_pos_latitude - destination.Data[destination.destNr+2-p].latitude))) 
                                    < 2.1*dest_dist)
                                && ((destination.Data[destination.destNr +2 -p].type == AV_DEST_TYPE_OCEANWYP)
                                    || (destination.Data[destination.destNr +2 -p].type == AV_DEST_TYPE_END)))
                        {

                            destination.longitude = destination.Data[destination.destNr +2 -p].longitude;
                            destination.latitude = destination.Data[destination.destNr +2 -p].latitude;
                            destination.destNr = (destination.destNr +2 -p);
                            destinationData.t_writefrom(destination);
skipperflags.skip_index_dest_call ++;
rtx_message("increased skip index to %d",skipperflags.skip_index_dest_call);

                            skipperflags.global_locator = AV_FLAGS_GLOBALSK_TRACKER;
                            skipperFlagData.t_writefrom(skipperflags);
                            break;
                        }
                        else if (p==2)
                        {
                            destination.longitude = destination.Data[destination.destNr].longitude;
                            destination.latitude = destination.Data[destination.destNr].latitude;
                            destinationData.t_writefrom(destination);
skipperflags.skip_index_dest_call ++;
                            skipperflags.global_locator = AV_FLAGS_GLOBALSK_CLOSING;
                            skipperFlagData.t_writefrom(skipperflags);
                        }
                    }


                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_GLOBALSK_CLOSING:

                    distance = (sqrt((current_pos_longitude - destination.longitude)
                                *(current_pos_longitude - destination.longitude)
                                + (current_pos_latitude - destination.latitude)
                                *(current_pos_latitude - destination.latitude)));
#ifdef DEBUG_GLOBSKIPPER
            rtx_message("closing: closest distance = %f \n", distance);
#endif
                    if(distance > 2.1*dest_dist)
                    {
                        destination.longitude = current_pos_longitude + 0.5 * (destination.longitude - current_pos_longitude);
                        destination.latitude = current_pos_latitude + 0.5 * (destination.latitude - current_pos_latitude);
                        destinationData.t_writefrom(destination);
skipperflags.skip_index_dest_call ++;
skipperFlagData.t_writefrom(skipperflags);
rtx_message("increased skip index to %d",skipperflags.skip_index_dest_call);
                    }

                    if(distance < dest_dist)
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                        skipperFlagData.t_writefrom(skipperflags);
                    }

                    break;
                    ////////////////////////////////////////////////////////////////////////////
                case AV_FLAGS_GLOBALSK_TRACKER:

#ifdef DEBUG_GLOBSKIPPER
                    rtx_message("distance to next destination = %f \n",  (sqrt((current_pos_longitude - destination.Data[destination.destNr].longitude)
                                    *(current_pos_longitude - destination.Data[destination.destNr].longitude)
                                    + (current_pos_latitude - destination.Data[destination.destNr].latitude)
                                    *(current_pos_latitude - destination.Data[destination.destNr].latitude))));

                    rtx_message("destinationtype = %d  \n", destination.Data[destination.destNr].type);
#endif
// rtx_message("distance to next destination = %f \n",  (sqrt((current_pos_longitude - destination.Data[destination.destNr].longitude)
//                                     *(current_pos_longitude - destination.Data[destination.destNr].longitude)
//                                     + (current_pos_latitude - destination.Data[destination.destNr].latitude)
//                                     *(current_pos_latitude - destination.Data[destination.destNr].latitude))));
                    if(((sqrt((current_pos_longitude - destination.Data[destination.destNr].longitude)
                                        *(current_pos_longitude - destination.Data[destination.destNr].longitude)
                                        + (current_pos_latitude - destination.Data[destination.destNr].latitude)
                                        *(current_pos_latitude - destination.Data[destination.destNr].latitude)))
                                < 0.2*dest_dist)
                            && (destination.Data[destination.destNr].type != AV_DEST_TYPE_END))
                    {
                        destination.destNr += 1;
// skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
// skipperFlagData.t_writefrom(skipperflags);

// rtx_message("increase_index_call to %d  \n", naviflags.navi_index_call);
                        assert((destination.destNr < 1000) && (destination.destNr>=0));
                        destination.longitude = destination.Data[destination.destNr].longitude;
                        destination.latitude = destination.Data[destination.destNr].latitude;
                        destinationData.t_writefrom(destination);
skipperflags.skip_index_dest_call ++;
skipperFlagData.t_writefrom(skipperflags);
rtx_message("increased skip index to %d",skipperflags.skip_index_dest_call);
                    }   

                    if((sqrt((current_pos_longitude - destination.Data[destination.destNr].longitude)
                                    *(current_pos_longitude - destination.Data[destination.destNr].longitude)
                                    + (current_pos_latitude - destination.Data[destination.destNr].latitude)
                                    *(current_pos_latitude - destination.Data[destination.destNr].latitude))) > 2.2*dest_dist)
                    {
			skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
skipperflags.skip_index_dest_call ++;
                        skipperFlagData.t_writefrom(skipperflags);
                    }

                    break;
                    /////////////////////////////////////////////////////////////////////////7    
                case AV_FLAGS_GLOBALSK_AVOIDANCE:

                    distance = (sqrt((current_pos_longitude - destination.longitude)
                                *(current_pos_longitude - destination.longitude)
                                + (current_pos_latitude - destination.latitude)
                                *(current_pos_latitude - destination.latitude)));
#ifdef DEBUG_GLOBSKIPPER
            rtx_message("Collision: collision distance = %f \n", distance);
#endif
// 		    if(first)
// 		    {
// 			skipperflags.skip_index_dest_call ++;
// skipperFlagData.t_writefrom(skipperflags);
// rtx_message("increased skip index to %d",skipperflags.skip_index_dest_call);
// 			first=false;
// 		    }
                    if((distance < 200/*.2*dest_dist*/) || (distance > 2*dest_dist))
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                        skipperFlagData.t_writefrom(skipperflags);
// 			first=true;
                    }

                    break;
                    ////////////////////////////////////////////////////////////////////////////
		case AV_FLAGS_GLOBALSK_SURVIVE:

                    distance = (sqrt((current_pos_longitude - destination.longitude)
                                *(current_pos_longitude - destination.longitude)
                                + (current_pos_latitude - destination.latitude)
                                *(current_pos_latitude - destination.latitude)));
#ifdef DEBUG_GLOBSKIPPER
            rtx_message("Collision: collision distance = %f \n", distance);
#endif

                    if((distance < 200/*.2*dest_dist*/) || (distance > 600))
                    {
                        skipperflags.global_locator = AV_FLAGS_GLOBALSK_LOCATOR;
                        skipperFlagData.t_writefrom(skipperflags);
                    }

                    break;
                    ////////////////////////////////////////////////////////////////////////////

            }



            //has to be modified:
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
