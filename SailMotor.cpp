/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	SailMotor.cpp	  Class Fuctions for the Sail Motor Class     */
/*									                                    */
/*	    Author  	      Stefan Wismer			                        */
/*                        wismerst@student.ethz.ch                      */
/*									                                    */
/************************************************************************/


#include <math.h>

#include "include/can.h"
#include "eposlib-test/epos.h"

#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include "avalon.h"
#include "SailMotor.h"

/**
 * Class functions
 * */

SailMotor::SailMotor() {}

SailMotor::~SailMotor() 
{
    rtx_message("SailMotor-Destructor has been called ... ");
	emergency_stop();
}

// Just clearing the errors and setting the operation Mode
bool SailMotor::init(const char * device) {

	can_init(device);

	epos_fault_reset(AV_SAIL_NODE_ID);
	epos_set_output_current_limit(AV_SAIL_NODE_ID, AV_MAX_SAIL_CURRENT);
	epos_set_continous_current_limit(AV_SAIL_NODE_ID, AV_MAX_SAIL_CONT_CURRENT);

	epos_shutdown(AV_SAIL_NODE_ID);
	epos_enable_operation(AV_SAIL_NODE_ID);

	epos_set_mode_of_operation(AV_SAIL_NODE_ID, EPOS_OPERATION_MODE_PROFILE_POSITION);

	epos_shutdown(AV_SAIL_NODE_ID);
	epos_enable_operation(AV_SAIL_NODE_ID);

	epos_set_profile_acceleration(AV_SAIL_NODE_ID, AV_SAIL_ACCELERATION);
	epos_set_profile_deceleration(AV_SAIL_NODE_ID, AV_SAIL_DECELERATION);
	epos_set_profile_velocity(AV_SAIL_NODE_ID, AV_SAIL_SPEED);

	epos_set_maximum_following_error(AV_SAIL_NODE_ID, AV_SAIL_MAX_FOLLOWING_ERROR);

    epos_set_brake_mask(AV_SAIL_NODE_ID, AV_SAIL_BRAKE_MASK);
    epos_set_brake_polarity(AV_SAIL_NODE_ID, AV_SAIL_BRAKE_POLARITY);

	printf("\n\nThe Sail motor at %s (CAN-ID %d) has the following status:\n", device, AV_SAIL_NODE_ID);
	epos_get_error(AV_SAIL_NODE_ID);

    epos_set_brake_state(AV_SAIL_NODE_ID, AV_SAIL_BRAKE_OPEN);

	return true;
}

bool SailMotor::calibrate(int id)
{
	// Set Operation Mode
	epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_HOMING);
	epos_set_homing_method(id, 0x0000);		// TODO: THIS HAS TO BE DEFINED AT SOME POINT

	// Set to Zero
	epos_start_homing_operation(id);

	// Reset Operation Mode
	epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_PROFILE_POSITION);
	
	epos_shutdown(id);
	epos_enable_operation(id);
	
	return true;
}

bool SailMotor::move_to_angle(float degrees, float reference, int& feedback, int& num_rounds) {
	
    float position;
    int target = 0;

	if(degrees <= 180 && degrees >= -180) {
		// get feedback
        epos_get_actual_position(AV_SAIL_NODE_ID);
        feedback = epos_read.node[AV_SAIL_NODE_ID-1].actual_position;
        position = remainder((feedback / (AV_SAIL_TICKS_PER_DEGREE)),360.0);

        num_rounds = (feedback - 180 * AV_SAIL_TICKS_PER_DEGREE) / (360 * AV_SAIL_TICKS_PER_DEGREE);
        if(fabs(degrees - position) > 180.0)
        {
            num_rounds -= 1*sign(remainder((position - degrees),360.0));
        }

		// Go to position
        target = (int)round(AV_SAIL_TICKS_PER_DEGREE * (degrees + reference + num_rounds*360));
        epos_set_target_position(AV_SAIL_NODE_ID, target);
		epos_activate_position(AV_SAIL_NODE_ID);
	}
	else {
		rtx_message("WARNING: A Sail angle beyond +-180 deg has been asked for. (at %f) Skipping...", degrees);
		return false;
	}

#if 0
	if(degrees <= AV_MAX_SAIL_ANGLE && degrees >= -AV_MAX_SAIL_ANGLE) {
		// Go to position
		epos_set_target_position(AV_SAIL_NODE_ID,(int)round((degrees+reference)*AV_SAIL_TICKS_PER_DEGREE));
		epos_activate_position(AV_SAIL_NODE_ID);

		// and get feedback
        epos_get_actual_position(AV_SAIL_NODE_ID);
        feedback = epos_read.node[AV_SAIL_NODE_ID-1].actual_position;
	}
	else {
		rtx_message("WARNING: A Sail angle beyond the max sail angle has been asked for. (at %f) Skipping...", degrees);
		return false;
	}
#endif

	return true;

}

bool SailMotor::emergency_stop() {

	epos_quick_stop(AV_SAIL_NODE_ID);
	return true;
}


// Some self-defined utility functions:
int SailMotor::sign(int i) // gives back the sign of an int
{
    if (i>=0)
        return 1;
    else 
        return -1;
}

int SailMotor::sign(float i) // gives back the sign of a float
{
    if (i>=0)
        return 1;
    else 
        return -1;
}

int SailMotor::sign(double i) // gives back the sign of a double
{
    if (i>=0)
        return 1;
    else 
        return -1;
}
