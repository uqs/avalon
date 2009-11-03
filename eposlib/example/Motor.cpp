/**
 *	This reads the target angles form the Store and sets it on the Rudder-EPOS
 *
 **/

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Specific Things
#include <can/epos.h>
#include <can/can.h>

#include "Motor.h"

Motor::Motor() {canid = 0;}

Motor::~Motor() {
	emergencyStop();
}

#define ACCELERATION 1000
#define DECELERATION 1000
#define SPEED 1000

// Just clearing the errors and setting the operation Mode
bool Motor::init(const std::string & device, unsigned int id) {

	canid = id;

	can_init(device.c_str());

	epos_fault_reset(canid);
	// epos_set_output_current_limit(canid, MAX_CURRENT);
	// epos_set_continous_current_limit(canid, MAX_CONT_CURRENT);

	epos_shutdown(canid);
	epos_enable_operation(canid);

	epos_set_mode_of_operation(canid, EPOS_OPERATION_MODE_PROFILE_POSITION);

	epos_shutdown(canid);
	epos_enable_operation(canid);

	epos_set_profile_acceleration(canid, ACCELERATION);
	epos_set_profile_deceleration(canid, DECELERATION);
	epos_set_profile_velocity(canid, SPEED);

	return true;
}

bool Motor::setPositionMode()
{
	epos_set_mode_of_operation(canid, EPOS_OPERATION_MODE_PROFILE_POSITION);
	return true;
}

bool Motor::setVelocityMode()
{
	epos_set_mode_of_operation(canid, EPOS_OPERATION_MODE_PROFILE_VELOCITY);
	return true;
}

bool Motor::moveToAngle(signed long ticks) {
	
	epos_set_target_position(canid,ticks);
	epos_activate_position(canid);
	return true;

}

bool Motor::moveToSpeed(signed long ticks) {
	
	epos_set_target_velocity(canid,ticks);
	epos_activate(canid);
	return true;

}

double Motor::getVelocity() {
	epos_get_actual_velocity(canid);
	return epos_read.node[canid-1].actual_velocity;
}

double Motor::getPosition() {
	epos_get_actual_position(canid);
	return epos_read.node[canid-1].actual_position;
}


bool Motor::emergencyStop() {

	epos_quick_stop(canid);
	return true;
}

