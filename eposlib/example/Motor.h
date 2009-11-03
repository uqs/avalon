/************************************************************************/
/*									*/
/*		       P R O J E K T    A V A L O N 			*/
/*								 	*/
/*	Motor.h	The Rudder-Motor class				*/
/*									*/
/*	Last Change	March 13, 2009; Stefan Wismer			*/
/*									*/
/************************************************************************/

#ifndef MOTOR_H
#define MOTOR_H

#include <string>

class Motor {

	public:
		Motor();
		~Motor();

		bool init(const std::string & device, unsigned int canid);

		bool calibrate();				// Sets the current position to 0 (per Motor)

		bool setPositionMode();

		bool setVelocityMode();

		bool moveToAngle(signed long ticks);	// Move only one Motor

		bool moveToSpeed(signed long tickpersec);	// Move only one Motor

		bool emergencyStop();

		double getVelocity();
		double getPosition();

	protected:
		unsigned int canid;

};

#endif // MOTOR_H
