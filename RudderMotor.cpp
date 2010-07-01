/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	RudderMotor.cpp	  Class Fuctions for the Rudder Motor Class     */
/*									                                    */
/*	    Author  	      Stefan Wismer			                        */
/*                        wismerst@student.ethz.ch                      */
/*									                                    */
/************************************************************************/


#include <math.h>

#include "can/can.h"
#include "epos/epos.h"

#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include "avalon.h"
#include "RudderMotor.h"

////// DEBUG STUFF /////////
#define SW_DEBUG
////////////////////////////


/**
 * Class functions
 * */

RudderMotor::RudderMotor() {

#ifdef SW_DEBUG
    printf("in constructor \n\n\n");
#endif // SW_DEBUG

}

RudderMotor::~RudderMotor() 
{
	emergency_stop();
    can_close();
}

// Just clearing the errors and setting the operation Mode
bool RudderMotor::init(int side, const char * device) {

	can_init(device);

    if(epos_test_bus(1) == 0)
    {
#ifdef SW_DEBUG
        printf("\nEPOS seems not to be at %s\nExiting...\n\n",device);
        return false;
#endif
    }
    
    int i;

	if(side == AV_RUDDER_LEFT) {
		i = 1;
	}
	else {
		i = 1;
	}


	epos_fault_reset(i);
	epos_set_output_current_limit(i, AV_MAX_RUDDER_CURRENT);
	epos_set_continous_current_limit(i, AV_MAX_RUDDER_CONT_CURRENT);

	epos_shutdown(i);
	epos_enable_operation(i);

	epos_set_mode_of_operation(i, EPOS_OPERATION_MODE_PROFILE_POSITION);

	epos_shutdown(i);
	epos_enable_operation(i);

	epos_set_profile_acceleration(i, AV_RUDDER_ACCELERATION);
	epos_set_profile_deceleration(i, AV_RUDDER_DECELERATION);
	epos_set_profile_velocity(i, AV_RUDDER_SPEED);

    // This is removed as following error is used to identify the epos
	// epos_set_maximum_following_error(i, AV_RUDDER_MAX_FOLLOWING_ERROR);

    epos_set_brake_state(i, AV_SAIL_BRAKE_OPEN);

    printf("\n\nThe %d. rudder motor at %s (CAN-ID %d, serial number %d) has the following status:\n", side, device, i, obtain_serial_number(i));
	epos_get_error(i);

	return true;
}

int RudderMotor::obtain_serial_number(int i)
{

    printf("\n\n");
    printf("The serial number of EPOS at Node %d is beeing read...\n", i);
   
    epos_get_maximum_following_error(i);

    printf("following is: %d\n", epos_read.node[i-1].maximum_following_error);

    return epos_read.node[i-1].maximum_following_error;
}


bool RudderMotor::conduct_homing(int id)
{
	// Set Operation Mode and homing method
	epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_HOMING);
	epos_set_homing_method(id, AV_HOMING_MODE);

    // Set All the Parameters
    epos_set_homing_speed_switch_search(id, AV_HOMING_SWITCH_SPEED);
    epos_set_homing_current_threshold(id, AV_HOMING_THRESHOLD);
    epos_set_homing_speed_zero_search(id, AV_HOMING_ZERO_SPEED);
    epos_set_home_position(id, AV_HOMING_POSITION);
    epos_set_home_offset(id, AV_HOMING_OFFSET);

	// Set to Zero
	epos_start_homing_operation(id);

	// Reset Operation Mode
	epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_PROFILE_POSITION);

	return true;
}

bool RudderMotor::move_to_angle(int id, float degrees, float reference, int& feedback) {
	
	if(degrees <= AV_MAX_RUDDER_ANGLE && degrees >= -AV_MAX_RUDDER_ANGLE) {
		// Go to position
		epos_set_target_position(id,(int)round((reference+degrees)*AV_RUDDER_TICKS_PER_DEGREE));
		epos_activate_position(id);

        // and get feedback
        epos_get_actual_position(1);
        feedback = epos_read.node[id-1].actual_position;
	}
	else {
		rtx_message("WARNING: A Rudder angle beyond the max rudder angle has been asked for. Skipping...");
		return false;
	}
	return true;

}

bool RudderMotor::move_to_angle(float degrees) {

	int i = 1;
	int dummy = 0;
    float fDummy = 0;
	while(i < AV_NOF_RUDDER_EPOS)
	{
		i++;
		move_to_angle(i, degrees, fDummy, dummy);

	}

	return true;
}

bool RudderMotor::emergency_stop() {

    rtx_message("RudderMotor-Destructor called...");
	// epos_quick_stop(1);
	return true;
}

