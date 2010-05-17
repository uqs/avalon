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

// General gsl-Things
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>

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
#include "sailor_rudder_iter_fn.h"
#include "sailor_main_iter_class.h"

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
    sailor_main_iter_class iter;


   //  double u;  // the input that will be written to rudderangle
    double theta_dot_des; //desired theta_dot by first controller
    double e;  // the current error
    RtxPid* mypid = NULL;
    RtxPid* thetapid = NULL;
    RtxPid* rudderpid = NULL;
    RtxParamStream* myparamstream = NULL;
    RtxParamStream* paramstream_theta_dot = NULL;
    RtxParamStream* paramstream_rudder = NULL;

    int sign_wanted_sail_angle = 1; // 1 or -1 depending on port or starboard driving
    int sign_wanted_rudder_angle = 1; // 1 or -1 depending on port or starboard driving before maneuver
    int sign_wanted_sail_angle_after_tack;
    int sign_wanted_sail_angle_after_change;
    int last_state;
    double sail_pre_jibe;
    double heading_pre_jibe;

    double wind_global_pre_tack;
    double wind_global_pre_jibe;
    double wind_global_pre_change;
    //double desired_heading_after_jibe;
    //double desired_bearing_after_jibe;
    double desired_heading_after_change;
    double desired_bearing_after_change;
    double wanted_sail_angle_after_change;
    double calculated_sail_degrees; 
    double desired_heading_after_tack;
    double desired_heading_while_no_tack_or_jibe;
    int last_no_tack_or_jibe_value;
    double torque_des;
    double u;
    double speed;


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

	    FILE * thetafile;
	    thetafile = fopen("thetaplot.txt","a+");
       
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
			    /* Sail: */
			    sign_wanted_sail_angle = sign(wind_clean.bearing_app);  // +1 for wind from starboard, -1 for port
			    avalon.wanted_sail_angle_to_wind = AV_SAILOR_WANTED_AOA * get_sail_AOA_coeff(wind_clean.speed);
			    // The following ensures that the sail is never set to the wrong side
			    // of closer than AV_SAILOR_UPWIND_MIN_SAIL_DEGREES
			    calculated_sail_degrees = remainder((wind_clean.bearing_app
						    - sign_wanted_sail_angle * avalon.wanted_sail_angle_to_wind),360.0);
			    if(sign_wanted_sail_angle * calculated_sail_degrees > AV_SAILOR_UPWIND_MIN_SAIL_DEGREES)
			    {
				    // slack the sail
				    sail.degrees = calculated_sail_degrees;
			    }
			    else
			    {
				    // set tight
				    sail.degrees = sign_wanted_sail_angle * AV_SAILOR_UPWIND_MIN_SAIL_DEGREES;
			    }

			    /* Torque: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(mypid);
			    }
			    /* Theta_dot: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_theta_dot = rtx_param_open("sailor_pid_theta_dot.txt", 0, NULL); //NULL = errorfunction
				    thetapid = rtx_pid_init(thetapid, paramstream_theta_dot, "theta_dot", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(thetapid);
			    }
			    /* Rudder: */ // at low speed
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_rudder = rtx_param_open("sailor_pidparams_old.txt", 0, NULL); //NULL = errorfunction
				    rudderpid = rtx_pid_init(rudderpid, paramstream_rudder, "rudder", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(rudderpid);
			    }
			    // compensate drift
			    if(imu_clean.velocity.x > 0.5) // below 0.5kn it probably doesn't make sense to compensate drift
			    {
				    desired_heading.heading = remainder(desired_heading.heading + atan2(imu_clean.velocity.y, imu_clean.velocity.x),360.0);
			    }
			    e = desired_heading.heading - imu.attitude.yaw;
			    if(fabs(e) > 180)
			    {
				    imu.attitude.yaw = 0;
				    desired_heading.heading = fabs(360.0 - fabs(e)) * sign(-e);
			    }
			    theta_dot_des = rtx_pid_eval(thetapid, imu.attitude.yaw, desired_heading.heading, 0);
                           
                            // theta_dot_des = imu.theta_star;
                            torque_des = rtx_pid_eval(mypid, imu.gyro.z, theta_dot_des, 0); //with p_max = I(3)/delta_t
                            rudder.torque_des = torque_des;
                            
// rtx_message("theta dot desired: %lf \n",theta_dot_des);
                            speed = 0.5144*sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));
                            if (speed <= 0.3)
                            {
                                    u = 0;//rtx_pid_eval(rudderpid, imu.attitude.yaw, desired_heading.heading, 0)*sign(imu.velocity.x);
                            }
                            else if((speed > 0.3) && (speed < 0.9))
                            {
                                    u = -iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des*0.05, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144); 
                            }
                            else if(speed >= 0.9) 
                            {
                                    u = iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144);
// rtx_message("headS: %f  T_des: %f  xS: %f  yS: %f  aoa: %f  dW: %f  z: %f  vW: %f\n",imu.gyro.z*M_PI/180.0, torque_des, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144);
			    }
// rtx_message("rudder degrees: %f",u*AV_PI/180.0);
			    fprintf(thetafile,"%f %f %f %f\n",theta_dot_des,imu.gyro.z,torque_des,rudder.degrees_left);
// rtx_message("NORM theta dot desired: %lf   torque_des: %lf    rudder: %lf\n",theta_dot_des, torque_des, u);
			    rudder.degrees_left = u;
			    rudder.degrees_right = u;
			    last_state = AV_FLAGS_ST_NORMALSAILING;
			    break;

		    case AV_FLAGS_ST_UPWINDSAILING:
			    /* Sail: */
			    sign_wanted_sail_angle = sign(wind_clean.bearing_app);  // +1 for wind from starboard, -1 for port
			    avalon.wanted_sail_angle_to_wind = AV_SAILOR_WANTED_AOA * get_sail_AOA_coeff(wind_clean.speed);
			    // The following ensures that the sail is never set to the wrong side
			    // of closer than AV_SAILOR_UPWIND_MIN_SAIL_DEGREES
			    calculated_sail_degrees = remainder((sign_wanted_sail_angle * AV_SAILOR_UPWIND_MIN_SAIL_DEGREES
						    - sign_wanted_sail_angle * (avalon.wanted_sail_angle_to_wind - AV_SAILOR_WANTED_AOA)),360.0);
			    if(sign_wanted_sail_angle * calculated_sail_degrees > AV_SAILOR_UPWIND_MIN_SAIL_DEGREES)
			    {
				    // slack the sail
				    sail.degrees = calculated_sail_degrees;
			    }
			    else
			    {
				    // set tight
				    sail.degrees = sign_wanted_sail_angle * AV_SAILOR_UPWIND_MIN_SAIL_DEGREES;
			    }

			    /* Rudder: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
			    }

			    /* Theta_dot: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_theta_dot = rtx_param_open("sailor_pid_theta_dot.txt", 0, NULL); //NULL = errorfunction
				    thetapid = rtx_pid_init(thetapid, paramstream_theta_dot, "theta_dot", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(thetapid);
			    }
			    /* Rudder: */ // at low speed
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_rudder = rtx_param_open("sailor_pidparams_old.txt", 0, NULL); //NULL = errorfunction
				    rudderpid = rtx_pid_init(rudderpid, paramstream_rudder, "rudder", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(rudderpid);
			    }
			    // redefine desired heading to stay "close to the wind":
			    desired_heading.heading = remainder(wind_clean.global_direction_real - sign_wanted_sail_angle * AV_SAILOR_MAX_HEIGHT_TO_WIND, 360.0);
			    e = desired_heading.heading - imu.attitude.yaw;
			    if(fabs(e) > 180) //TODO: check: why not remainder???
			    {
				    imu.attitude.yaw = 0;
				    desired_heading.heading = fabs(360.0 - fabs(e)) * sign(-e);
			    }
                            
                            theta_dot_des = rtx_pid_eval(thetapid, imu.attitude.yaw, desired_heading.heading, 0) ;
                            //theta_dot_des = imu.theta_star;
                            torque_des = rtx_pid_eval(mypid, imu.gyro.z, theta_dot_des, 0); //with p_max = I(3)/delta_t
                            rudder.torque_des = torque_des;      
                            
                            speed = 0.5144*sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));
                            if (speed <= 0.3)
                            {
                                    u = 0;//rtx_pid_eval(rudderpid, imu.attitude.yaw, desired_heading.heading, 0)*sign(imu.velocity.x);
                            }
                            else if((speed > 0.3) && (speed < 0.9))
                            {
                                    u = -iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des*0.05, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144); 
                            }
                            else if(speed >= 0.9) 
                            {
                                    u = iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144); 
                                    //rtx_message("u_zero = %f \n",u);
			    }                          
// rtx_message("UPWI theta dot desired: %lf   torque_des: %lf    rudder: %lf\n",theta_dot_des, torque_des, u);
			    fprintf(thetafile,"%f %f %f %f\n",theta_dot_des,imu.gyro.z,torque_des,rudder.degrees_left);
			    rudder.degrees_left = u;
			    rudder.degrees_right = u;
			    last_state = AV_FLAGS_ST_UPWINDSAILING;
			    break;

		    case AV_FLAGS_ST_DOWNWINDSAILING:
			    /* Sail: */
			    sign_wanted_sail_angle = sign(wind_clean.bearing_app);  // +1 for wind from starboard, -1 for port
			    avalon.wanted_sail_angle_to_wind = AV_SAILOR_WANTED_AOA * get_sail_AOA_coeff(wind_clean.speed);
			    calculated_sail_degrees = remainder((wind_clean.bearing_app
						    - sign_wanted_sail_angle * avalon.wanted_sail_angle_to_wind),360.0);
			    if(sign_wanted_sail_angle * calculated_sail_degrees > 0) // if still on "correct" side
			    {
				    // slack the sail
				    sail.degrees = calculated_sail_degrees;
			    }
			    else
			    {
				    // set to 179 deg
				    sail.degrees = sign_wanted_sail_angle * 179.0;
			    }
			    sail.degrees = sign_wanted_sail_angle * AV_SAILOR_DOWNWIND_SAIL_DEGREES;

			    /* Torque: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
			    }

			    /* Theta_dot: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_theta_dot = rtx_param_open("sailor_pid_theta_dot.txt", 0, NULL); //NULL = errorfunction
				    thetapid = rtx_pid_init(thetapid, paramstream_theta_dot, "theta_dot", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(thetapid);
			    }
			    /* Rudder: */ // at low speed
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_rudder = rtx_param_open("sailor_pidparams_old.txt", 0, NULL); //NULL = errorfunction
				    rudderpid = rtx_pid_init(rudderpid, paramstream_rudder, "rudder", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(rudderpid);
			    }
			    // redefine desired heading to stay fixed to the wind:
			    desired_heading.heading = remainder(wind_clean.global_direction_real - sign_wanted_sail_angle * AV_SAILOR_MAX_DOWNWIND_ANGLE, 360.0);
			    e = desired_heading.heading - imu.attitude.yaw;
			    if(fabs(e) > 180) // take care of +-180 thing
			    {
				    imu.attitude.yaw = 0;
				    desired_heading.heading = fabs(360.0 - fabs(e)) * sign(-e);
			    }

			    theta_dot_des = rtx_pid_eval(thetapid, imu.attitude.yaw, desired_heading.heading, 0) ;
                            torque_des = rtx_pid_eval(mypid, imu.gyro.z, theta_dot_des, 0); //with p_max = I(3)/delta_t
                            rudder.torque_des = torque_des;                            
			 
                            speed = 0.5144*sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));
                            if (speed <= 0.3)
                            {
                                    u = 0;//rtx_pid_eval(rudderpid, imu.attitude.yaw, desired_heading.heading, 0)*sign(imu.velocity.x);
                            }
                            else if((speed > 0.3) && (speed < 0.9))
                            {
                                    u = -iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des*0.05, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144); 
                            }
                            else if(speed >= 0.9) 
                            {
                                    u = iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144);
                                    //rtx_message("u_zero = %f \n",u);
			    }

			    fprintf(thetafile,"%f %f %f %f\n",theta_dot_des,imu.gyro.z,torque_des,rudder.degrees_left);
			    rudder.degrees_left = u;
			    rudder.degrees_right = u;
			    last_state = AV_FLAGS_ST_DOWNWINDSAILING;
			    break;

		    case AV_FLAGS_ST_TACK:
			    if(last_state != flags.state) // only when newly in this state, otherwise rudder angle changes in the middle of the tack
			    {
				    sign_wanted_rudder_angle = -sign(wind_clean.bearing_app);  // +1 for wind from starboard, -1 for port
				    wind_global_pre_tack = wind_clean.global_direction_real;
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
				    desired_heading_after_tack =
					    remainder(wind_clean.global_direction_real - 45.0 *
							    sign(remainder(imu.attitude.yaw -
									    wind_clean.global_direction_real,360.0)),360.0);
				    sign_wanted_sail_angle_after_tack = -sign(wind_clean.bearing_app);
			    }
			    /* Torque: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(mypid);
			    }

			    /* Theta_dot: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_theta_dot = rtx_param_open("sailor_pid_theta_dot.txt", 0, NULL); //NULL = errorfunction
				    thetapid = rtx_pid_init(thetapid, paramstream_theta_dot, "theta_dot", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(thetapid);
			    }
			    /* Rudder: */ // at low speed
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_rudder = rtx_param_open("sailor_pidparams_old.txt", 0, NULL); //NULL = errorfunction
				    rudderpid = rtx_pid_init(rudderpid, paramstream_rudder, "rudder", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(rudderpid);
			    }
			    /* Sail: */
			    // if heading is still on 'wrong' side:
			    if(remainder((wind_global_pre_tack - imu.attitude.yaw),360.0) * sign_wanted_sail_angle_after_tack < 0)
			    {
				    sail.degrees = AV_SAILOR_UPWIND_MIN_SAIL_DEGREES / AV_SAILOR_MAX_HEIGHT_TO_WIND * remainder((wind_global_pre_tack - imu.attitude.yaw),360.0);
			    }
			    else // sail on correct (for after tack) side
			    {
				    sail.degrees = sign_wanted_sail_angle_after_tack * AV_SAILOR_UPWIND_MIN_SAIL_DEGREES;
			    }

			    /* Rudder: */
			    //u = sign_wanted_sail_angle * 45.0 * sign(imu.velocity.x);

			    /* the following is only for Q-tacks instead of jibes */
			    //imu.attitude.yaw = remainder((imu.attitude.yaw - wind_global_pre_tack),360.0);
			    //desired_heading_after_tack = remainder((desired_heading_after_tack - wind_global_pre_tack),360.0);
			    /* end Q-tacks... */

			    // Attention: if() mit e is missing(see normal sailing...) probably not needed though !!!!!!!!
			    // u = rtx_pid_eval(mypid, imu.attitude.yaw, desired_heading_after_tack, 0) * sign(imu.velocity.x);

			    theta_dot_des = rtx_pid_eval(thetapid, imu.attitude.yaw, desired_heading_after_tack, 0) ;
                            torque_des = rtx_pid_eval(mypid, imu.gyro.z, theta_dot_des, 0); //with p_max = I(3)/delta_t                            
                            rudder.torque_des = torque_des;
                             
                            speed = 0.5144*sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));
                            if (speed <= 0.3)
                            {
                                    u = 0;//rtx_pid_eval(rudderpid, imu.attitude.yaw, desired_heading.heading, 0)*sign(imu.velocity.x);
                            }
                            else if((speed > 0.3) && (speed < 0.9))
                            {
                                    u = -iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des*0.05, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144); 
                            }
                            else if(speed >= 0.9) 
                            {
                                    u = iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144);
                                    
			    }
// 			    rtx_message("u_zero = %f \n",u);
// 			    rtx_message("des_head = %f, head = %f \n",desired_heading_after_tack,imu.attitude.yaw);
// rtx_message("TACK  theta dot desired: %lf   torque_des: %lf    rudder: %lf\n",theta_dot_des, torque_des, u);
			    fprintf(thetafile,"%f %f %f %f\n",theta_dot_des,imu.gyro.z,torque_des,rudder.degrees_left);
// rtx_message("rudder degrees: %f",u*AV_PI/180.0);
			    rudder.degrees_left = u;
			    rudder.degrees_right = u;
			    last_state = AV_FLAGS_ST_TACK;
			    break;

		    case AV_FLAGS_ST_JIBE:
			    if(last_state != flags.state) // only when newly in this state
			    {
                                    sail_pre_jibe = sailstate.degrees_sail;
                                    heading_pre_jibe = imu.attitude.yaw;
				    wind_global_pre_jibe = wind_clean.global_direction_real;
				  //  desired_heading_after_jibe = fabs(remainder((wind_clean.global_direction_real
				//				    + AV_SAILOR_MAX_DOWNWIND_ANGLE * sign(wind_clean.bearing_real)),360.0));
				  //  desired_bearing_after_jibe = remainder((wind_global_pre_jibe - desired_heading_after_jibe),360.0);
				    sign_wanted_sail_angle = sign(wind_clean.bearing_real); // +1 for wind from starboard, -1 for port
			    }

			    /* Sail: */
			    if(sailstate.degrees_sail * sign_wanted_sail_angle <= 0 || fabs(sailstate.degrees_sail) > 170.0)
				    // sail already on "correct" side or turned out to the front
			    {
				    sail.degrees = -sign_wanted_sail_angle * AV_SAILOR_DOWNWIND_SAIL_DEGREES;
			    }
			    else //sail still on "wrong/old" side
			    {
				    sail.degrees = 180.0; // set to front
			    }

			    /* Torque: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(mypid);
			    }

			    /* Theta_dot: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_theta_dot = rtx_param_open("sailor_pid_theta_dot.txt", 0, NULL); //NULL = errorfunction
				    thetapid = rtx_pid_init(thetapid, paramstream_theta_dot, "theta_dot", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(thetapid);
			    }
			    /* Rudder: */ // at low speed
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_rudder = rtx_param_open("sailor_pidparams_old.txt", 0, NULL); //NULL = errorfunction
				    rudderpid = rtx_pid_init(rudderpid, paramstream_rudder, "rudder", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(rudderpid);
			    }
			    if(sailstate.degrees_sail * sign_wanted_sail_angle <= 0) // Sail already on correct side
			    {
				    // desiredheading in function of sailstate.degrees_sail...
				    desired_heading.heading = remainder(remainder((wind_global_pre_jibe + 180.0),360.0) 
							    - sign_wanted_sail_angle * ((180-AV_SAILOR_MAX_DOWNWIND_ANGLE)  
							    * fabs(fabs(sailstate.degrees_sail) - 180.0) / fabs(AV_SAILOR_DOWNWIND_SAIL_DEGREES - 180.0)),360.0);
			    }
			    else // sail still on wrong side
			    {
				    desired_heading.heading = remainder(remainder((wind_global_pre_jibe + 180.0),360.0) 
							    - (remainder((remainder(wind_global_pre_jibe + 180,360.0) - heading_pre_jibe),360.0)  
                                                            * (fabs(sailstate.degrees_sail) - 180.0) / (fabs(sail_pre_jibe) - 180.0)),360.0);
			    }

			    e = desired_heading.heading - imu.attitude.yaw;
			    if(fabs(e) > 180) // +-180 thing...
			    {
				    imu.attitude.yaw = 0;
				    desired_heading.heading = fabs(360.0 - fabs(e)) * sign(-e);

			    }

                            theta_dot_des = rtx_pid_eval(thetapid, imu.attitude.yaw, desired_heading.heading, 0) ;
                            torque_des = rtx_pid_eval(mypid, imu.gyro.z, theta_dot_des, 0); //with p_max = I(3)/delta_t
                            rudder.torque_des = torque_des;          

                            speed = 0.5144*sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));
                            if (speed <= 0.3)
                            {
                                    u = 0;//rtx_pid_eval(rudderpid, imu.attitude.yaw, desired_heading.heading, 0)*sign(imu.velocity.x);
                            }
                            else if((speed > 0.3) && (speed < 0.9))
                            {
                                    u = -iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des*0.05, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144); 
                            }
                            else if(speed >= 0.9) 
                            {
                                    u = iter.sailor_main_iter_fn(imu.gyro.z*M_PI/180.0, torque_des, imu_clean.velocity.x*0.5144, -imu_clean.velocity.y*0.5144, sailstate.degrees_sail*M_PI/180.0, wind_clean.global_direction_real*M_PI/180.0, imu.attitude.yaw*M_PI/180.0, wind_clean.speed*0.5144);
			            //rtx_message("u_zero = %f \n",u);
			    }

			    fprintf(thetafile,"%f %f %f %f\n",theta_dot_des,imu.gyro.z,torque_des,rudder.degrees_left);
// rtx_message("rudder degrees: %f",u*AV_PI/180.0);
			    rudder.degrees_left = u;
			    rudder.degrees_right = u;
			    last_state = AV_FLAGS_ST_JIBE;
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

			    /* Sail: */
			    switch(flags.sail_direction)
			    {
				    case AV_FLAGS_SAIL_DIR_NOPREFERENCE:
					    sail.degrees = wanted_sail_angle_after_change;
					    break;
				    case AV_FLAGS_SAIL_DIR_ZERO:
					    sail.degrees = 0.0;
					    break;
				    case AV_FLAGS_SAIL_DIR_FRONT:
					    sail.degrees = 179.0;
					    break;
			    }

			    /* Rudder: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    // TODO maybe use different parameters here (I=0)
				    myparamstream = rtx_param_open("sailor_pid_torque.txt", 0, NULL); //NULL = errorfunction
				    mypid = rtx_pid_init(mypid, myparamstream, "torque", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(mypid);
			    }

			    /* Theta_dot: */
			    if(last_state != flags.state) // initialize only when newly in this state
			    {
				    paramstream_theta_dot = rtx_param_open("sailor_pid_theta_dot.txt", 0, NULL); //NULL = errorfunction
				    thetapid = rtx_pid_init(thetapid, paramstream_theta_dot, "theta_dot", 0.01, 0); //0.01=dt
				    rtx_pid_integral_enable(thetapid);
			    }

			    desired_heading.heading = remainder((wind_clean.global_direction_real 
						    - avalon.wanted_sail_angle_to_wind * sign(sailstate.degrees_sail) 
						    - sailstate.degrees_sail),360.0);

			    e = desired_heading.heading - imu.attitude.yaw;
			    if(fabs(e) > 180) // +-180 thing...
			    {
				    imu.attitude.yaw = 0;
				    desired_heading.heading = fabs(360.0 - fabs(e)) * sign(-e);
			    }

                            theta_dot_des = rtx_pid_eval(thetapid, imu.attitude.yaw, desired_heading.heading, 0) ;

                            torque_des = rtx_pid_eval(mypid, imu.gyro.z, theta_dot_des, 0); //with p_max = I(3)/delta_t                          
                            rudder.torque_des = torque_des;                            
                            

			    // rtx_message("des_heading_dot = %f, des_torque = %f \n",theta_dot_des,rudder.torque_des);
			    // rtx_message("des_head = %f, head = %f \n",desired_heading.heading,imu.attitude.yaw);
			   //  rudder.degrees_left = u;
			   //  rudder.degrees_right = u;
			    last_state = AV_FLAGS_ST_HEADINGCHANGE;
			    break;

		    default:
			    rtx_message("Sailor state machine is in some illegal state. Behaving as in IDLE --> stopping motion");
			    sail.degrees = sailstate.degrees_sail;
			    last_state = 0;
			    break;
	    }

	    fclose(thetafile);
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
