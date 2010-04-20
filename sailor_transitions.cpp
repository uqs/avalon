/************************************************************************/
/*									                                    */
/*		       P R O J E K T    A V A L O N 			                */
/*								 	                                    */
/*	sailor_transitions.cpp  This sets the different sailor states       */
/*									                                    */
/*	Last Change	May 14th, 2009; Hendrik Erckens	                		*/
/*									                                    */
/************************************************************************/
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

#include <DDXStore.h>
#include <DDXVariable.h>

// Specific Things
#include "flags.h"
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
DDXVariable dataFlags;
DDXVariable dataSailorFlags;
DDXVariable dataSailState;
DDXVariable dataWindClean;
DDXVariable dataImu;
DDXVariable dataImuClean;
DDXVariable dataDesiredHeading;

/**
 * Prototypes for self-defined utility functions
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
const char * varname_windDataClean = "cleanwind";
const char * varname_sailstate = "sailstate";
const char * varname_imu = "imu";
const char * varname_imuClean = "cleanimu";
const char * varname_desiredheading = "desiredheading";
const char * producerHelpStr = "This is the Sailor's transitions state machine";

/**
 * Command line arguments
 * */
RtxGetopt producerOpts[] =
{
    {"flagsname", "Store-Variable where the data from the flags are",
        {
            {RTX_GETOPT_STR, &varname_flags, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"sailorflagsname", "Store Variable for sailor flags",
        {
            {RTX_GETOPT_STR, &varname_sailorflags, ""},
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
    Flags flags;
    sailorFlags sailorflags;
    Ship avalon;
    Sailstate sailstate;
    WindCleanData wind_clean;
    imuData imu;
	imuCleanData imu_clean;
    DesiredHeading desired_heading;
    double desired_heading_after_tack;
    double desired_heading_after_jibe;
    double desired_heading_after_change;
    double wind_global_pre_jibe;
    double wind_global_pre_tack;
    double wind_global_pre_change;
    double desired_bearing_after_jibe;
    double desired_bearing_after_change;
    int sign_wanted_sail_angle;
    int sign_wanted_sail_angle_after_change;
    double wanted_sail_angle_after_jibe;
    double wanted_sail_angle_after_change;
    int last_state = 0;
    time_t tacktimeout_start, tacktimeout_currenttime;
    double tacktimeout_diff;
    time_t tackendtimeout_start, tackendtimeout_currenttime;
    double tackendtimeout_diff;
    time_t jibetimeout_start, jibetimeout_currenttime;
    double jibetimeout_diff;
    time_t jibeendtimeout_start, jibeendtimeout_currenttime;
    double jibeendtimeout_diff;
    time_t headingchangeendtimeout_start, headingchangeendtimeout_currenttime;
    double headingchangeendtimeout_diff;


    while (1)
    {
        dataFlags.t_readto(flags,0,0);
        dataSailorFlags.t_readto(sailorflags,0,0);
        dataImu.t_readto(imu,0,0);
        dataSailState.t_readto(sailstate,0,0);
        dataImuClean.t_readto(imu_clean,0,0);
        dataWindClean.t_readto(wind_clean,0,0);
        dataDesiredHeading.t_readto(desired_heading,0,0);

        // calculate desired heading with drift compensation
        if(imu_clean.velocity.x > 0.5) // below 0.5kn it probably doesn't make sense to compensate drift
        {
            desired_heading.heading = remainder(desired_heading.heading + atan2(imu_clean.velocity.drift, imu_clean.velocity.x),360.0);
        }

        // reset no_tack_or_jibe flag
        sailorflags.no_tack_or_jibe = 0;

        switch(flags.state)
        {
            case AV_FLAGS_ST_IDLE: //current state IDLE
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_IDLE:
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in IDLE",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_IDLE;
                break;

            case AV_FLAGS_ST_DOCK: //current state DOCK
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_DOCK:
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in DOCK",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_DOCK;
                break;

            case AV_FLAGS_ST_NORMALSAILING: //current state NORMALSAILING
                // to UPWINDSAILING
// rtx_message("desired_head= %lf   wind_clean= %lf \n",desired_heading.heading,wind_clean.global_direction_real );
		if(fabs(remainder((desired_heading.heading - wind_clean.global_direction_real),360.0)) <= 45.0) //AV_SAILOR_MAX_HEIGHT_TO_WIND
                {
                    sailorflags.state = AV_FLAGS_ST_UPWINDSAILING;
                }
                // to TACK
                time(&tacktimeout_currenttime);
                tacktimeout_diff = difftime(tacktimeout_currenttime, tacktimeout_start); // yields time in [s]
                if((fabs(wind_clean.bearing_real) < 90.0) 
                        && (remainder((imu.attitude.yaw - wind_clean.global_direction_real),360.0) 
                         * remainder((desired_heading.heading + sign(remainder(desired_heading.heading - wind_clean.global_direction_real,360.0))
                         * AV_SAILOR_TACK_HYSTERESIS - wind_clean.global_direction_real),360.0) < 0))
                {
                    if(tacktimeout_diff > 20.0) // next tack only if 20 seconds after the previous
                    {
                        sailorflags.state = AV_FLAGS_ST_TACK;
                        time(&tackendtimeout_start); // start the timer
                    }
                    else // tell statemachine not to tack/jibe without TACK/JIBE...
                    {
                        // set the flag. It is being reset to 0 at the very top of
                        // the while loop
                        sailorflags.no_tack_or_jibe = 1;
                    }
                }
                // to DOWNWINDSAILING
                if(fabs(remainder((desired_heading.heading - wind_clean.global_direction_real),360.0)) >= AV_SAILOR_MAX_DOWNWIND_ANGLE)
                {
                    sailorflags.state = AV_FLAGS_ST_DOWNWINDSAILING;
                }
                // to JIBE
                time(&jibetimeout_currenttime);
                jibetimeout_diff = difftime(jibetimeout_currenttime, jibetimeout_start); // yields time in [s]
                if((fabs(wind_clean.bearing_real) > 90.0)
                        && (remainder((imu.attitude.yaw - wind_clean.global_direction_real),360.0) 
                         * remainder((desired_heading.heading - sign(remainder(desired_heading.heading - wind_clean.global_direction_real,360.0))
					 * AV_SAILOR_JIBE_HYSTERESIS - wind_clean.global_direction_real),360.0) < 0) && (sign(sailstate.degrees_sail)*sign(wind_clean.bearing_real)>0))
                {
                    if(jibetimeout_diff > 20.0) // next jibe only if 20 seconds after the previous
                    {
                        sailorflags.state = AV_FLAGS_ST_JIBE;
                        //sailorflags.state = AV_FLAGS_ST_TACK;
                        time(&jibeendtimeout_start); // start the timer, will kick boat out of JIBE after x seconds
                    }
                    else // tell statemachine not to tack/jibe without TACK/JIBE...
                    {
                        // set the flag. It is being reset to 0 at the very top of
                        // the while loop
                        sailorflags.no_tack_or_jibe = 1;
                    }
                }
                // to HEADINGCHANGE
                if(fabs(remainder((desired_heading.heading - imu.attitude.yaw),360.0)) > AV_SAILOR_HEADINGCHANGE_ERROR)
                    // maybe add wind speed condition...
                {
                    if(sailorflags.state == AV_FLAGS_ST_TACK
                            && wind_clean.bearing_real > 80.0)
                    {
                        sailorflags.sail_direction = AV_FLAGS_SAIL_DIR_ZERO;
                        sailorflags.state = AV_FLAGS_ST_HEADINGCHANGE;
                    }
                    else if(sailorflags.state == AV_FLAGS_ST_JIBE
                            && wind_clean.bearing_real < 130.0)
                    {
                        sailorflags.sail_direction = AV_FLAGS_SAIL_DIR_FRONT;
                        sailorflags.state = AV_FLAGS_ST_HEADINGCHANGE;
                    }
                    else
                    {
                        sailorflags.sail_direction = AV_FLAGS_SAIL_DIR_NOPREFERENCE;
                        sailorflags.state = AV_FLAGS_ST_HEADINGCHANGE;
                    }
                    time(&headingchangeendtimeout_start); // start the timer, will kick boat out of HEADINGCHANGE after x seconds
                }
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in NORMALSAILING",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_NORMALSAILING;
                break;

            case AV_FLAGS_ST_UPWINDSAILING: //current state UPWINDSAILING
                // to NORMALSAILING
                if(fabs(remainder((desired_heading.heading - wind_clean.global_direction_real),360.0)) > AV_SAILOR_MAX_HEIGHT_TO_WIND)
                {
                    sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                } 
                // to TACK
                time(&tacktimeout_currenttime);
                tacktimeout_diff = difftime(tacktimeout_currenttime, tacktimeout_start); // yields time in [s]
                if((fabs(wind_clean.bearing_real) < 90.0) 
                        && (remainder((imu.attitude.yaw - wind_clean.global_direction_real),360.0) 
                        * remainder((desired_heading.heading + sign(remainder(desired_heading.heading - wind_clean.global_direction_real,360.0))
                        * AV_SAILOR_TACK_HYSTERESIS - wind_clean.global_direction_real),360.0) < 0))
                {
                    if(tacktimeout_diff > 20.0) // next tack only if 20 seconds after the previous
                    {
                        sailorflags.state = AV_FLAGS_ST_TACK;
                        time(&tackendtimeout_start); // start the timer
                    }
                    else // tell statemachine not to tack/jibe without TACK/JIBE...
                    {
                        // set the flag. It is being reset to 0 at the very top of
                        // the while loop
                        sailorflags.no_tack_or_jibe = 1;
                    }
                }

                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_UPWINDSAILING:
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in UPWINDSAILING",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_UPWINDSAILING;
                break;

             case AV_FLAGS_ST_DOWNWINDSAILING: //current state DOWNWINDSAILING
                // to NORMALSAILING
                if(fabs(remainder((desired_heading.heading - wind_clean.global_direction_real),360.0)) < AV_SAILOR_MAX_DOWNWIND_ANGLE)
                {
                    sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                }
                // to JIBE
                time(&jibetimeout_currenttime);
                jibetimeout_diff = difftime(jibetimeout_currenttime, jibetimeout_start); // yields time in [s]
                if((fabs(wind_clean.bearing_real) > 90.0)
                        && (remainder((imu.attitude.yaw - wind_clean.global_direction_real),360.0) 
                        * remainder((desired_heading.heading - sign(remainder(desired_heading.heading - wind_clean.global_direction_real,360.0))
                        * AV_SAILOR_JIBE_HYSTERESIS - wind_clean.global_direction_real),360.0) < 0))
                {
                    if(jibetimeout_diff > 20.0) // next jibe only if 20 seconds after the previous
                    {
                        sailorflags.state = AV_FLAGS_ST_JIBE;
                        time(&jibeendtimeout_start); // start the timer, will kick boat out of JIBE after x seconds
                    }
                    else // tell statemachine not to tack/jibe without TACK/JIBE...
                    {
                        // set the flag. It is being reset to 0 at the very top of
                        // the while loop
                        sailorflags.no_tack_or_jibe = 1;
                    }
                }
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_UPWINDSAILING:
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in DOWNWINDSAILING",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_DOWNWINDSAILING;
                break;

           case AV_FLAGS_ST_TACK: //current state TACK
                if(last_state != flags.state) //only when new in this state
                {
                    desired_heading_after_tack = remainder(wind_clean.global_direction_real 
                                - AV_SAILOR_MAX_HEIGHT_TO_WIND * sign(remainder(imu.attitude.yaw 
                                - wind_clean.global_direction_real,360.0)),360.0);
                    wind_global_pre_tack = wind_clean.global_direction_real; // for HEADINGCHANGE
                }
// rtx_message("des_head = %f, head = %f \n",desired_heading_after_tack,imu.attitude.yaw);
                time(&tackendtimeout_currenttime);
                tackendtimeout_diff = difftime(tackendtimeout_currenttime, tackendtimeout_start); // yields time in [s]
                // to NORMALSAILING
                if((fabs(remainder((desired_heading_after_tack - imu.attitude.yaw),360.0)) < AV_SAILOR_EPSILON_TACK)
                        || (tackendtimeout_diff > AV_SAILOR_TACK_END_TIMEOUT))
                {
rtx_message("change to normalsailing");
                    sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                    time(&tacktimeout_start); // start the timer
                }
                switch(flags.state_requested)
                {
                    case 0:
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in TACK",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_TACK;
                break;

            case AV_FLAGS_ST_JIBE: //current state JIBE
                if(last_state != flags.state) //only when new in this state
                {
                    // set desired heading after jibe all the way to "the other side"
                    wind_global_pre_jibe = wind_clean.global_direction_real;
                    desired_heading_after_jibe = fabs(remainder((wind_clean.global_direction_real
                                    + AV_SAILOR_MAX_DOWNWIND_ANGLE * sign(wind_clean.bearing_real)),360.0));
                    desired_bearing_after_jibe = remainder((wind_global_pre_jibe - desired_heading_after_jibe),360.0);
                    sign_wanted_sail_angle = sign(desired_bearing_after_jibe); // +1 for wind from starboard, -1 for port
                    wanted_sail_angle_after_jibe = sign_wanted_sail_angle * AV_SAILOR_DOWNWIND_SAIL_DEGREES;
                }
                // to NORMALSAILING
                time(&jibeendtimeout_currenttime);
                jibeendtimeout_diff = difftime(jibeendtimeout_currenttime, jibeendtimeout_start); // yields time in [s]
                if((fabs(remainder((wanted_sail_angle_after_jibe - sailstate.degrees_sail),360.0)) < AV_SAILOR_EPSILON_JIBE)
                        || (jibeendtimeout_diff > AV_SAILOR_JIBE_END_TIMEOUT))
                {
                    sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                    time(&jibetimeout_start); // start the timer
                }
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in JIBE",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_JIBE;
                break;

            case AV_FLAGS_ST_MAXENERGYSAVING: // current state MAXENERGYSAVING
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in MAXENERGYSAVING",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_MAXENERGYSAVING;
                break;

            case AV_FLAGS_ST_HEADINGCHANGE: // current state HEADINGCHANGE
                if(last_state != flags.state) // only when newly in this state
                {
                    switch(last_state)
                    {
                        case AV_FLAGS_ST_TACK: // use old wind direction
                            wind_global_pre_change = wind_global_pre_tack;
                            break;
                        case AV_FLAGS_ST_JIBE: // use old wind direction
                            wind_global_pre_change = wind_global_pre_jibe;
                            break;
                        default: // get current wind reading
                            wind_global_pre_change = wind_clean.global_direction_real;
                    }
                    avalon.wanted_sail_angle_to_wind = AV_SAILOR_WANTED_AOA * get_sail_AOA_coeff(wind_clean.speed);
                    desired_heading_after_change = desired_heading.heading;
                    desired_bearing_after_change = remainder((wind_clean.global_direction_real - desired_heading_after_change),360.0);
                    sign_wanted_sail_angle_after_change = sign(desired_bearing_after_change);
                    wanted_sail_angle_after_change = remainder((desired_bearing_after_change - avalon.wanted_sail_angle_to_wind * sign_wanted_sail_angle),360.0);
                }
                // to NORMALSAILING
                time(&headingchangeendtimeout_currenttime);
                headingchangeendtimeout_diff = difftime(headingchangeendtimeout_currenttime, headingchangeendtimeout_start); // yields time in [s]
                if((fabs(remainder((wanted_sail_angle_after_change - sailstate.degrees_sail),360.0)) < AV_SAILOR_EPSILON_HEADINGCHANGE)
                        || (headingchangeendtimeout_diff > AV_SAILOR_HEADINGCHANGE_END_TIMEOUT))
                {
                    sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                }
                // to UPWINDSAILING
                if(fabs(sailstate.degrees_sail) < AV_SAILOR_UPWIND_MIN_SAIL_DEGREES)
                {
                    sailorflags.state = AV_FLAGS_ST_UPWINDSAILING;
                }
                // to DOWNWINDSAILING
                if(fabs(sailstate.degrees_sail) > AV_SAILOR_DOWNWIND_SAIL_DEGREES)
                {
                    sailorflags.state = AV_FLAGS_ST_DOWNWINDSAILING;
                }
                switch(flags.state_requested)
                {
					case 0:
						break;
                    case AV_FLAGS_ST_HEADINGCHANGE:
                        break;
                    case AV_FLAGS_ST_MAXENERGYSAVING:
                        sailorflags.state = AV_FLAGS_ST_MAXENERGYSAVING;
                        break;
                    case AV_FLAGS_ST_IDLE:
                        sailorflags.state = AV_FLAGS_ST_IDLE;
                        break;
                    case AV_FLAGS_ST_DOCK:
                        sailorflags.state = AV_FLAGS_ST_DOCK;
                        break;
                    case AV_FLAGS_ST_NORMALSAILING:
                        sailorflags.state = AV_FLAGS_ST_NORMALSAILING;
                        break;
                    default:
                        rtx_message("illegal state request %d. Staying in HEADINGCHANGE",flags.state_requested);
                }
                last_state = AV_FLAGS_ST_HEADINGCHANGE;
                break;

            default:
                rtx_message("current sailor state %d is illegal. Switching to IDLE",flags.state_requested);
                sailorflags.state = AV_FLAGS_ST_IDLE;
        }

        dataSailorFlags.t_writefrom(sailorflags);
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

int main (int argc, const char * argv[])
{
    RtxThread * th;
    int ret;

    // Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1)
    {
        RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
        exit (1);
    }
    rtx_main_init ("Sailor Transitions Main", RTX_ERROR_STDERR);

    // Open the store
    DOB(store.open());

    // Register the new Datatypes
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), sailorFlags));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Sailstate));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DesiredHeading));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuCleanData));

    // Connect to existing variables, or create new variables
    DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
    DOB(store.registerVariable(dataSailorFlags, varname_sailorflags, "sailorFlags"));
    DOB(store.registerVariable(dataWindClean, varname_windDataClean, "WindCleanData"));
    DOB(store.registerVariable(dataSailState, varname_sailstate, "Sailstate"));
	DOB(store.registerVariable(dataImu, varname_imu, "imuData"));
    DOB(store.registerVariable(dataDesiredHeading, varname_desiredheading, "DesiredHeading"));
	DOB(store.registerVariable(dataImuClean, varname_imuClean, "imuCleanData"));

    // Start the working thread
    DOP(th = rtx_thread_create ("Sailor Transitions thread", 0,
                                RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
                                RTX_THREAD_CANCEL_DEFERRED,
                                translation_thread, NULL,
                                NULL, NULL));

    // Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
    rtx_message_routine ("Ctrl-C detected. Shutting down Sailor Transitions...");

    // Terminating the thread
    rtx_thread_destroy_sync (th);

    // The destructors will take care of cleaning up the memory
    return (0);
}
