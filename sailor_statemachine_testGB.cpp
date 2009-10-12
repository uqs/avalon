/************************************************************************/
/*									                                    */
/*		       P R O J E K T    A V A L O N 			                */
/*								 	                                    */
/*	sailor_statemachine.cpp  This is the sailor state machine           */
/*									                                    */
/*	Last Change	April 12, 2009; Hendrik Erckens	                		*/
/*									                                    */
/************************************************************************/
//bla to test git, and again, and yet again
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
#include <rtx/pid.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "flags.h"
#include "sail-target.h"
#include "rudder-target.h"
#include "Ship.h"
#include "Sailstate.h"
#include "Rudderstate.h"
#include "windcleaner.h"
#include "imu.h"
#include "imucleaner.h"
#include "desired_course.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataRudder;
DDXVariable dataSail;
DDXVariable dataFlags;
DDXVariable dataSailorFlags;
DDXVariable dataSailState;
DDXVariable dataRudderStateLeft;
DDXVariable dataRudderStateRight;
DDXVariable dataWindClean;
DDXVariable dataImu;
DDXVariable dataImuClean;
DDXVariable dataDesiredHeading;

/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(float i);
int sign(double i);
double get_sail_AOA_coeff(double roll);

/**
 * Storage for the command line arguments
 * */
const char * varname_flags = "flags";
const char * varname_sailorflags = "sailorflags";
const char * varname_ruddertarget = "rudder";
const char * varname_sailtarget = "sail";
const char * varname_windDataClean = "cleanwind";
const char * varname_sailstate = "sailstate";
const char * varname_rudderstateleft = "rudderstateleft";
const char * varname_rudderstateright = "rudderstateright";
const char * varname_imu = "imu";
const char * varname_imuClean = "cleanimu";
const char * varname_desiredheading = "desiredheading";
const char * producerHelpStr = "This is the Ship's skipper";

/**
 * Command line arguments
 * */
RtxGetopt producerOpts[] =
{
    {"flagsname", "Store-Variable where the flags are",
        {
            {RTX_GETOPT_STR, &varname_flags, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"sailorflagsname", "Store-Variable where the sailor flags are",
        {
            {RTX_GETOPT_STR, &varname_sailorflags, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"ruddername", "Store Variable where the rudder-angle target is written",
        {
            {RTX_GETOPT_STR, &varname_ruddertarget, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"sailname", "Store Variable where the sail-angle target is written",
        {
            {RTX_GETOPT_STR, &varname_sailtarget , ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"cleanwindname", "Store Variable where clean wind data is",
        {
            {RTX_GETOPT_STR, &varname_windDataClean, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"sailstatename", "Store Variable where sail state is",
        {
            {RTX_GETOPT_STR, &varname_sailstate, ""},
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
    {"imuname", "Store Variable where the imuData is",
        {
            {RTX_GETOPT_STR, &varname_imu, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"cleanimuname", "Store Variable where the cleaned imu data is written to",
        {
            {RTX_GETOPT_STR, &varname_imuClean, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"desiredheadingname", "Store Variable where the desired heading is",
        {
            {RTX_GETOPT_STR, &varname_desiredheading, ""},
            RTX_GETOPT_END_ARG
        }
    },
    RTX_GETOPT_END
};


/**
 * Working thread, wait for the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{
    rudderTarget rudder = {0.0, 0.0, 0, 0};
    sailTarget sail = {0.0, 0};
    Flags flags;
    sailorFlags sailorflags;
    Ship avalon;
    Sailstate sailstate;
    Rudderstate rudderstateleft, rudderstateright;
    WindCleanData wind_clean;
    imuData imu;
	imuCleanData imu_clean;
    DesiredHeading desired_heading;
    
    double u;  // the input that will be written to rudderangle
    double e;  // the current error
    RtxPid* mypid = NULL;
    RtxParamStream* myparamstream = NULL;

    int sign_wanted_sail_angle = 1; // 1 or -1 depending on port or starboard driving
    int sign_wanted_rudder_angle = 1; // 1 or -1 depending on port or starboard driving before maneuver
    int sign_wanted_sail_angle_after_tack;
    int sign_wanted_sail_angle_after_change;
    int last_state;
    double wind_global_pre_tack;
    double wind_global_pre_jibe;
    double wind_global_pre_change;
    double desired_heading_after_jibe;
    double desired_bearing_after_jibe;
    double desired_heading_after_change;
    double desired_bearing_after_change;
    double wanted_sail_angle_after_change;
    double calculated_sail_degrees; 
    double desired_heading_after_tack;
    double desired_heading_while_no_tack_or_jibe;
    int last_no_tack_or_jibe_value;

    while (1)
    {
        dataFlags.t_readto(flags,0,0);
        dataSailorFlags.t_readto(sailorflags,0,0);
        if (flags.man_in_charge != AV_FLAGS_MIC_SAILOR) // someone else is in charge...
        {
            rtx_timer_sleep(0.1);
            continue;   // don't do anything if someone else is in charge
        }
        // wait 10 sec for IMU data
        if (dataImu.t_readto(imu,10.0,1)) 
		{
            dataWindClean.t_readto(wind_clean,0,0);
            dataImuClean.t_readto(imu_clean,0,0);
			dataSailState.t_readto(sailstate,0,0);
			dataRudderStateLeft.t_readto(rudderstateleft,0,0);
			dataRudderStateRight.t_readto(rudderstateright,0,0);
			dataDesiredHeading.t_readto(desired_heading,0,0);

            if(flags.sailor_no_tack_or_jibe == 1)
            {
                if(last_no_tack_or_jibe_value == 0)
                {
                    // set desired heading to current heading before beginning
                    // the wait during no_tack_or_jibe
                    desired_heading_while_no_tack_or_jibe = imu.attitude.yaw;
                }
                desired_heading.heading = desired_heading_while_no_tack_or_jibe;
            }

            if(flags.state == AV_FLAGS_ST_NORMALSAILING
                    || flags.state == AV_FLAGS_ST_UPWINDSAILING
                    || flags.state == AV_FLAGS_ST_DOWNWINDSAILING
                    || flags.state == AV_FLAGS_ST_TACK
                    || flags.state == AV_FLAGS_ST_JIBE
                    || flags.state == AV_FLAGS_ST_HEADINGCHANGE)
            {
                flags.state = AV_FLAGS_ST_NORMALSAILING;
            }
       
            switch(flags.state)
            {
                case AV_FLAGS_ST_IDLE:
                    rtx_timer_sleep(0.1);
                    last_state = AV_FLAGS_ST_IDLE;
                    continue;   // don't do anything
                    break;

                case AV_FLAGS_ST_DOCK:
                    /* Sail: */
                    avalon.wanted_sail_angle_to_wind = 0;
                    sail.degrees = remainder((wind_clean.bearing_app
                                - sign_wanted_sail_angle * avalon.wanted_sail_angle_to_wind),360.0);

                    /* Rudder: */
                    rudder.degrees_left = -45.0;
                    rudder.degrees_right = 45.0;
                    last_state = AV_FLAGS_ST_DOCK;
                    break;

                case AV_FLAGS_ST_NORMALSAILING:
                    /* Rudder: */
                    if(last_state != flags.state) // initialize only when newly in this state
                    {
                        myparamstream = rtx_param_open("sailor_pidparams.txt", 0, NULL); //NULL = errorfunction
                        mypid = rtx_pid_init(mypid, myparamstream, "rudder", 0.01, 0); //0.01=dt
                        rtx_pid_integral_enable(mypid);
                    }
                    e = desired_heading.heading - imu.attitude.yaw;
                    if(fabs(e) > 180)
                    {
                        imu.attitude.yaw = 0;
                        desired_heading.heading = fabs(360.0 - fabs(e)) * sign(-e);
                    }
                    u = rtx_pid_eval(mypid, imu.attitude.yaw, desired_heading.heading, 0) * sign(imu_clean.velocity.x);
                    if(imu.velocity.x > AV_SAILOR_DECREASE_RUDDER_THRESHOLD)
                    {
                        // rudder angle is decreased if speed is high
                        u = u * AV_SAILOR_DECREASE_RUDDER_THRESHOLD / imu_clean.velocity.x;
                    }
                    rudder.degrees_left = u;
                    rudder.degrees_right = u;
                    last_state = AV_FLAGS_ST_NORMALSAILING;
                    break;

                case AV_FLAGS_ST_UPWINDSAILING:
                    break;

                 case AV_FLAGS_ST_DOWNWINDSAILING:
                    break;

               case AV_FLAGS_ST_TACK:
                    break;

                case AV_FLAGS_ST_JIBE:
                    break;

                case AV_FLAGS_ST_MAXENERGYSAVING:
                    /* Sail: */
                    sail.degrees = sign(sailstate.degrees_sail) * 179.0;
                    /* Rudder: */
                    rudder.degrees_left = -45.0;
                    rudder.degrees_right = 45.0;
                    last_state = AV_FLAGS_ST_MAXENERGYSAVING;
                    break;

                case AV_FLAGS_ST_HEADINGCHANGE:
                    break;

                default:
                    rtx_message("Sailor state machine is in some illegal state. Behaving as in IDLE --> stopping motion");
                    sail.degrees = sailstate.degrees_sail;
                    last_state = 0;
                    break;
            }

            // Bring to store
            dataRudder.t_writefrom(rudder);
            dataSail.t_writefrom(sail);
            // Do not write to sailorflags in here! Sailor_transitions does that.

        // Take care of timeouts:
        } else if (dataWindClean.hasTimedOut())
        {
        rtx_message("Timeout while reading windClean data.\n");
        } else if (dataSailState.hasTimedOut())
        {
        rtx_message("Timeout while reading Sailstate data.\n");
        } else if (dataRudderStateLeft.hasTimedOut())
        {
        rtx_message("Timeout while reading left Rudderstate data.\n");
        } else if (dataRudderStateRight.hasTimedOut())
        {
        rtx_message("Timeout while reading right Rudderstate data.\n");
        } else if (dataImuClean.hasTimedOut())
        {
        rtx_message("Timeout while reading ImuClean data.\n");
        }

        // rtx_timer_sleep(0.1);
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

int sign(double i) // gives back the sign of a double
{
    if (i>=0)
        return 1;
    else 
        return -1;
}

double get_sail_AOA_coeff(double roll)
{
    // 0.7∙1/(1+exp(0.2(abs(x)−23)))+0.3
    return (0.7*(1 / (1 + exp(0.2 * (fabs(roll) - 26))))+0.3);
    // = 0.5 * cos((imu_clean.attitude.roll + 17) / 30) + 0.5 - 0.12 * cos((imu_clean.attitude.roll + 17) / 14) + 0.12;
}

int main (int argc, char * argv[])
{
    RtxThread * th;
    int ret;

    // Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1)
    {
        RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
        exit (1);
    }
    rtx_main_init ("Sailor Statemachine Main", RTX_ERROR_STDERR);

    // Open the store
    DOB(store.open());

    // Register the new Datatypes
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailorFlags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rudderTarget));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailTarget));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Sailstate));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Rudderstate));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DesiredHeading));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuCleanData));


    // Connect to existing variables, or create new variables
    DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
    DOB(store.registerVariable(dataSailorFlags, varname_sailorflags, "sailorFlags"));
    DOB(store.registerVariable(dataRudder, varname_ruddertarget, "rudderTarget"));
    DOB(store.registerVariable(dataSail, varname_sailtarget, "sailTarget"));
    DOB(store.registerVariable(dataWindClean, varname_windDataClean, "WindCleanData"));
    DOB(store.registerVariable(dataSailState, varname_sailstate, "Sailstate"));
    DOB(store.registerVariable(dataRudderStateLeft, varname_rudderstateleft, "Rudderstate"));
    DOB(store.registerVariable(dataRudderStateRight, varname_rudderstateright, "Rudderstate"));
    DOB(store.registerVariable(dataDesiredHeading, varname_desiredheading, "DesiredHeading"));
	DOB(store.registerVariable(dataImu, varname_imu, "imuData"));
	DOB(store.registerVariable(dataImuClean, varname_imuClean, "imuCleanData"));

    // Start the working thread
    DOP(th = rtx_thread_create ("Sailor Statemachine thread", 0,
                                RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
                                RTX_THREAD_CANCEL_DEFERRED,
                                translation_thread, NULL,
                                NULL, NULL));

    // Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
    rtx_message_routine ("Ctrl-C detected. Shutting down sailor_statemachine Routine...");

    // Terminating the thread
    rtx_thread_destroy_sync (th);

    // The destructors will take care of cleaning up the memory
    return (0);
}
