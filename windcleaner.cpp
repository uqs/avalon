/**
 *	This converts the Joy-Stick signals into sail- and rudder angles
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
#include "Sailstate.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWind;
DDXVariable dataWindClean;
DDXVariable dataSailState;
DDXVariable dataBoat;

/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(float i);

/**
 * Storage for the command line arguments
 * */
const char * varname1 = "wind";
const char * varname2 = "cleanwind";
const char * varname3 = "sailstate";
const char * varname4 = "imu";
const char * producerHelpStr = "Cleaning up the wind data from the sensor";

/**
 * Command line arguments
 *
 * */
RtxGetopt producerOpts[] = {
  {"innameWind", "Store-Variable where the dirty wind data comes from",
   {
     {RTX_GETOPT_STR, &varname1, "WindData"},
     RTX_GETOPT_END_ARG
   }
  },
  {"out", "Store Variable where the cleaned Wind data is written to",
   {
     {RTX_GETOPT_STR, &varname2, "WindCleanData"},
     RTX_GETOPT_END_ARG
   }
  },
  {"sailname", "Store Variable where the current sail-angle is written",
   {
     {RTX_GETOPT_STR, &varname3, "Sailstate"},
     RTX_GETOPT_END_ARG
   }
  },
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
    WindData dirtyWind;
    WindCleanData cleanedWind, workingWind;
	WindCleanData lastWindData[100];
	imuData boatData;
	Sailstate sailData;
	double bearing_app_X, bearing_app_Y;
	double speed_app_X, speed_app_Y;
	int nenner, nenner_long, count=0;
    // int first_wind_sign;
    // bool firsttime=true;

    cleanedWind.speed=0.0;
    cleanedWind.speed_long=0.0;
    cleanedWind.angle_of_attack_app=0.0;
    cleanedWind.bearing_app=0.0;
    cleanedWind.bearing_real=0.0;
    cleanedWind.global_direction_app=0.0;
    cleanedWind.global_direction_real=0.0;
    cleanedWind.global_direction_real_long=0.0;

	dataWindClean.t_writefrom(cleanedWind);

    for (int i=0;i<100;i++)
    {
        lastWindData[i].speed= 0.0;
        lastWindData[i].speed_long= 0.0;
        lastWindData[i].angle_of_attack_app = 0.0;
        lastWindData[i].bearing_app = 0.0;
        lastWindData[i].bearing_real = 0.0;
        lastWindData[i].global_direction_app = 0.0;
        lastWindData[i].global_direction_real = 0.0;
        lastWindData[i].global_direction_real_long = 0.0;
    }

    while (1) {


	    FILE * windfile;

 if(count < 10000)
 {	    
 	    windfile = fopen("windplot.txt","a+");
 }
 if(count == 10000)
 { 
   count = 0;
   windfile = fopen("windplot.txt","w+");
 }
count ++;	    
	    // Read the next data available, or wait at most 5 seconds
	    if (dataWind.t_readto(dirtyWind,10.0,1))
	    {
		    dataWindClean.t_readto(cleanedWind,0,0);  //might be dispensible????
		    dataSailState.t_readto(sailData,0,0);
		    dataBoat.t_readto(boatData,0,0);
#if 0
		    if(firsttime && dirtyWind.direction != 0)
		    {
			    first_wind_sign = sign(dirtyWind.direction);
			    firsttime = false;
		    }
#endif //never

		    //workingWind.global_direction_real_long has to be defined because
		    //its not beeing used!!!

		    workingWind.global_direction_real_long = 0.0;
		    workingWind.speed_long = 0.0;

		    /**shift back the last calculations**/

		    for (int i=0;i<99;i++)
		    {
			    lastWindData[i]=lastWindData[i+1];
		    }

		    /**calculating angle of attack apparent in workingWind**/

		    workingWind.angle_of_attack_app = remainder((double) dirtyWind.direction,360.0);
		    //rtx_message("AOA = %f\n",dirtyWind.direction); 

		    /** bearing apparent:**/

		    workingWind.bearing_app = remainder(workingWind.angle_of_attack_app + (sailData.degrees_sail), 360.0);

		    /**bearing real:**/

		    bearing_app_X = dirtyWind.speed * cos(workingWind.bearing_app * AV_PI / 180.0 ); //attention: here we used apparent wind speed, is that correct????
		    bearing_app_Y = dirtyWind.speed * sin(workingWind.bearing_app * AV_PI / 180.0 );
		    // with drift:
// 		    workingWind.bearing_real = remainder(atan2((bearing_app_Y + boatData.velocity.y),(bearing_app_X - boatData.velocity.x))*180/AV_PI,360);
		    // without drift
		    workingWind.bearing_real = remainder(atan2((bearing_app_Y),(bearing_app_X - boatData.velocity.x))*180/AV_PI,360);
		    /**global real:**/

		    workingWind.global_direction_real = remainder((workingWind.bearing_real + boatData.attitude.yaw),360.0);

		    /**global apparent:**/

		    workingWind.global_direction_app = remainder((workingWind.bearing_app + boatData.attitude.yaw),360.0);

		    /**calculating the correct speed**/

		    speed_app_X = dirtyWind.speed * cos(workingWind.bearing_app * AV_PI / 180.0);
		    speed_app_Y = dirtyWind.speed * sin(workingWind.bearing_app * AV_PI / 180.0);
// rtx_message("Windspeed in boat system: x= %f  y= %f",speed_app_X, speed_app_Y);
		     // with drift:
// 		    workingWind.speed = sqrt(pow((speed_app_X - boatData.velocity.x),2) + pow((speed_app_Y + boatData.velocity.y),2));
		    // without drift
		    workingWind.speed = sqrt(pow((speed_app_X - boatData.velocity.x),2) + pow((speed_app_Y),2));

		    /**doing the right thing with the new values to avoid leaps (-180 to 180) - for every value, there is two operations:**/

		    //angle of attack
		    if(fabs(remainder(lastWindData[98].angle_of_attack_app,360.0) - workingWind.angle_of_attack_app) > 180.0)
		    {
			    workingWind.angle_of_attack_app += sign((float) remainder(lastWindData[98].angle_of_attack_app,360.0)) * 360.0;
		    }
		    workingWind.angle_of_attack_app += (lastWindData[98].angle_of_attack_app - remainder(lastWindData[98].angle_of_attack_app,360.0));

		    //bearing app
		    if(fabs(remainder(lastWindData[98].bearing_app,360.0) - workingWind.bearing_app) > 180.0)
		    {
			    workingWind.bearing_app +=sign((float) remainder(lastWindData[98].bearing_app,360.0))*360.0;
		    }
		    workingWind.bearing_app += (lastWindData[98].bearing_app - remainder(lastWindData[98].bearing_app,360.0));

		    //bearing real        
		    if(fabs(remainder(lastWindData[98].bearing_real,360.0) - workingWind.bearing_real) > 180.0)
		    {
			    workingWind.bearing_real += sign((float) remainder(lastWindData[98].bearing_real,360.0))*360.0;
		    }
		    workingWind.bearing_real += (lastWindData[98].bearing_real - remainder(lastWindData[98].bearing_real,360.0));

		    //global direction app
		    if(fabs(remainder(lastWindData[98].global_direction_app,360.0) - workingWind.global_direction_app) > 180.0)
		    {
			    workingWind.global_direction_app += sign((float) remainder(lastWindData[98].global_direction_app,360.0))*360.0;
		    }
		    workingWind.global_direction_app += (lastWindData[98].global_direction_app - remainder(lastWindData[98].global_direction_app,360.0));

		    //global direction real
		    if(fabs(remainder(lastWindData[98].global_direction_real,360.0) - workingWind.global_direction_real) > 180.0)
		    {
			    workingWind.global_direction_real += sign((float) remainder(lastWindData[98].global_direction_real,360.0))*360.0;
		    }
		    workingWind.global_direction_real += (lastWindData[98].global_direction_real - remainder(lastWindData[98].global_direction_real,360.0));


		    //putting the working wind into the last array-values
		    lastWindData[99].speed = workingWind.speed;
		    lastWindData[99].angle_of_attack_app = workingWind.angle_of_attack_app;
		    lastWindData[99].bearing_app = workingWind.bearing_app;
		    lastWindData[99].bearing_real = workingWind.bearing_real;
		    lastWindData[99].global_direction_app = workingWind.global_direction_app;
		    lastWindData[99].global_direction_real = workingWind.global_direction_real;


		    //set cleanedWind to zero:
		    cleanedWind.speed=0.0;
		    cleanedWind.speed_long=0.0;
		    cleanedWind.angle_of_attack_app=0.0;
		    cleanedWind.bearing_app=0.0;
		    cleanedWind.bearing_real=0.0;
		    cleanedWind.global_direction_app=0.0;
		    cleanedWind.global_direction_real = 0.0;
		    cleanedWind.global_direction_real_long = 0.0;

		    nenner =(int) (0.5 * AV_CLEAN_LENGTH * (1 + AV_CLEAN_LENGTH));
		    nenner_long =(int)(0.5 * AV_CLEAN_LENGTH_LONG * (1 + AV_CLEAN_LENGTH_LONG));


		    for (int i=0;i<AV_CLEAN_LENGTH;i++ )
		    {
			    cleanedWind.speed += (double (i+1))/((double) nenner)*lastWindData[100-AV_CLEAN_LENGTH+i].speed;
			    //cleanedWind.angle_of_attack_app += (double (i+1))/((double) nenner)*lastWindData[100-AV_CLEAN_LENGTH+i].angle_of_attack_app;
			    cleanedWind.bearing_app += (double (i+1))/((double) nenner)*lastWindData[100-AV_CLEAN_LENGTH+i].bearing_app;
			    cleanedWind.bearing_real += (double (i+1))/((double) nenner)*lastWindData[100-AV_CLEAN_LENGTH+i].bearing_real;
			    cleanedWind.global_direction_app += (double (i+1))/((double) nenner)*lastWindData[100-AV_CLEAN_LENGTH+i].global_direction_app;
			    cleanedWind.global_direction_real += (double (i+1))/((double) nenner)*lastWindData[100-AV_CLEAN_LENGTH+i].global_direction_real;
		    }

		    for (int i=0;i<AV_CLEAN_LENGTH_LONG;i++)
		    {
			    cleanedWind.global_direction_real_long += (double (i+1))/((double) nenner_long)*lastWindData[100-AV_CLEAN_LENGTH_LONG+i].global_direction_real;
			    cleanedWind.speed_long += (double (i+1))/((double) nenner_long)*lastWindData[100-AV_CLEAN_LENGTH_LONG+i].speed;
		    }

		    //rtx_message("AOA = %f \n",cleanedWind.angle_of_attack_app);
		    //cleanedWind.angle_of_attack_app=remainder(cleanedWind.angle_of_attack_app,360.0);
		    cleanedWind.bearing_app=remainder(cleanedWind.bearing_app,360.0);
		    cleanedWind.bearing_real=remainder(cleanedWind.bearing_real,360.0);
		    cleanedWind.global_direction_app=remainder(cleanedWind.global_direction_app,360.0);
		    cleanedWind.global_direction_real = remainder(cleanedWind.global_direction_real,360.0);
		    cleanedWind.global_direction_real_long = remainder(cleanedWind.global_direction_real_long, 360.0);

		    /**angle of attack apparent calculated from global_direction apparent**/

		    cleanedWind.angle_of_attack_app = remainder((-sailData.degrees_sail - boatData.attitude.yaw + cleanedWind.global_direction_app),360.0);


		    // Bring to store
		    dataWindClean.t_writefrom(cleanedWind);
		    
		    fprintf(windfile,"%f %f %f %f %f %f \n", workingWind.global_direction_real,cleanedWind.global_direction_real, cleanedWind.global_direction_real_long, workingWind.speed, cleanedWind.speed, cleanedWind.speed_long);

	    }

	    else if (dataWind.hasTimedOut()) {
		    // Timeout. Probably no joystick connected.

		    rtx_message("Timeout while reading data from the windsensor.cpp. ist the windsensor connected? \n");}

	    else if (dataSailState.hasTimedOut()) {
		    // Timeout. Probably no joystick connected.

		    rtx_message("Timeout while reading data about sailangle from Shipstate.cpp. ist the EPOS connected? \n");}

	    else if (dataWindClean.hasTimedOut()) {
		    // Timeout. Probably no joystick connected.

		    rtx_message("Timeout while reading dataWindClean \n");}

	    else if (dataBoat.hasTimedOut()) {
		    // Timeout. Probably no joystick connected.

		    rtx_message("Timeout while reading IMU-Data \n");}

	    else {
		    // Something strange happend. Critical Error.
		    rtx_error("Critical error while reading data");
		    // Emergency-Stop
		    rtx_main_signal_shutdown();
	    }
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



int main (int argc, const char * argv[])
{
	RtxThread * th;
	int ret;

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Clean Wind interface Main", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Sailstate));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));


	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataWindClean, varname2, "WindCleanData"));
	DOB(store.registerVariable(dataWind, varname1, "WindData"));
	DOB(store.registerVariable(dataSailState, varname3, "Sailstate"));
	DOB(store.registerVariable(dataBoat, varname4, "imuData"));

	// Start the working thread
	DOP(th = rtx_thread_create ("Wind cleaning thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
				RTX_THREAD_CANCEL_DEFERRED,
				translation_thread, NULL,
				NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down windcleaner...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
	return (0);
}
