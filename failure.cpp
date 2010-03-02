/**
 * failure management program!!
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
#include "Rudderstate.h"
#include "rudder-target.h"
//#include "weatherdata.h"


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
DDXVariable errorFlagData;
DDXVariable destinationStruct;
DDXVariable destinationData; //the actual values!
DDXVariable headingData; //the curr heading will be written here; sailor needs that!!!
DDXVariable dataRudder;
DDXVariable dataRudderStateLeft;
DDXVariable dataRudderStateRight;
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
const char * varname_errorflags = "errorflags";
const char * varname_transf = "transfData";

const char * varname_destStruct = "destStruct";
const char * varname_destData = "destData";

const char * varname_course = "desiredheading";

const char * varname = "rudder";
const char * varname_rudderstateleft = "rudderstateleft";
const char * varname_rudderstateright = "rudderstateright";
//const char * varname_weather = "weather";


const char * producerHelpStr = "failure detection help-string";

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
  {"inname", "Store-Variable where the target angle is",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
    {"rudderstateleftname", "Store Variable where left rudder state is",
        {
            {RTX_GETOPT_STR, &varname_rudderstateleft, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"rudderstaterightname", "Store Variable where right rudder state is",
        {
            {RTX_GETOPT_STR, &varname_rudderstateright, ""},
            RTX_GETOPT_END_ARG
        }
    },
  {"errorflags", "Store Variable where the flags are",
   {
     {RTX_GETOPT_STR, &varname_errorflags, ""},
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
    //rcFlags rcflags;
    //Destination destinationCoord; 
    DestinationData destination; //actual array!!
    DesiredHeading desiredHeading; //course that goes to HE
    rudderTarget rudder;
    Rudderstate rudderstateleft, rudderstateright;
    ErrorFlags errorflags;
    
    // double torque_des;
    double I_z;
    double delta_t;

    double gyroHistory[19];
    double gyroPredictionHistory[19];
    double longHistory[19];
    double latHistory[19];

    double sum_gyroHistory;
    double sum_gyroPredictionHistory;
    double average_gyroHistory;
    double average_gyroPredictionHistory;
    double average_long;
    double average_lat;
    double error_long;
    double error_lat;	
    double error_gyro;
    //dataRudderState.t_writefrom(rudderstate);

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
            //dataRcFlags.t_readto(rcflags,0,0);
            dataRudder.t_readto(rudder,0,0);
            dataRudderStateLeft.t_readto(rudderstateleft,0,0);
            dataRudderStateRight.t_readto(rudderstateright,0,0);
            errorFlagData.t_readto(errorflags,0,0);

            delta_t                       = 0.1;
            I_z                           = 150;	    
            sum_gyroHistory               = 0;
            average_gyroHistory           = 0;
            sum_gyroPredictionHistory     = 0;
            average_gyroPredictionHistory = 0;
            average_long                  = 0;
            average_lat                   = 0;
            error_long                    = 0;
            error_lat                     = 0;
            error_gyro                    = 0;
   
	    
	    gyroHistory[18]               = boatData.gyro.z;
            gyroPredictionHistory[18]     = boatData.gyro.z;
            longHistory[18]               = boatData.position.longitude;
            latHistory[18]                = boatData.position.latitude;

            //shift back all the history:
            for(int i=0; i<19; i++)
	    {
		    gyroHistory[i]            = gyroHistory[i+1];
                    gyroPredictionHistory[i]  = gyroPredictionHistory[i+1];
                    longHistory[i]            = longHistory[i+1];
                    latHistory[i]             = latHistory[i+1];
           
	    }	    //fill new data:
	    gyroHistory[19]                   = boatData.gyro.z;
            gyroPredictionHistory[19]         = rudder.torque_des*delta_t/I_z + gyroHistory[18];
            longHistory[19]                   = boatData.position.longitude;
            latHistory[19]                    = boatData.position.latitude;

	    //hier kannst du jetzt mitteln oder alles machen was du willst mit den daten...:
            for (int j=1; j<20; j++)
            {
                  sum_gyroHistory                 += gyroHistory[j];
                  average_gyroHistory             += 1.0/19.0 * gyroHistory[j];
                  sum_gyroPredictionHistory       += gyroPredictionHistory[j];
                  average_gyroPredictionHistory   += 1.0/19.0 * gyroPredictionHistory[j];
                  average_long                    += 1.0/19.0 * longHistory[j];
                  average_lat                     += 1.0/19.0 * latHistory[j];
                  error_long                       = average_long;  // not thisone (imagine all values = 0)   += fabs( longHistory[j]-longHistory[j-1] );
                  error_lat                        = average_lat;                                    //       += fabs( latHistory[j] -latHistory[j-1] );
                  error_gyro            = sqrt(average_gyroPredictionHistory*average_gyroPredictionHistory) - sqrt(average_gyroHistory*average_gyroHistory);
               
            }

            if (fabs(error_gyro) > 50)
            {
                  errorflags.state = AV_FLAGS_ERROR_SYSTEM_ID;
            }

            if ( (error_long > 150) || (error_lat > 150) )
            {
                  errorflags.state = AV_FLAGS_ERROR_IMU;
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
	rtx_main_init ("failure detection Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
        DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ErrorFlags));
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
    DOP(th = rtx_thread_create ("failure detection thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down failure detection...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);

}
