/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	RudderMotor.h	  The Rudder-Motor class				        */
/*									                                    */
/*	    Author  	      Stefan Wismer			                        */
/*                        wismerst@student.ethz.ch                      */
/*									                                    */
/************************************************************************/


#ifndef RUDDERMOTOR_H
#define RUDDERMOTOR_H

class RudderMotor {

public:
	RudderMotor();
	~RudderMotor();

	bool init(int side, const char* device);

    int obtain_serial_number(int i);

	bool conduct_homing_left(int id);				// Sets the current position to 0 (per Motor)
	bool conduct_homing_right(int id);				// Sets the current position to 0 (per Motor)

	bool move_to_angle(float degrees);		// Move both Motors
	bool move_to_angle(int id, float degrees, float reference, int& feedback);	// Move only one Motor

	bool emergency_stop();
	

protected:

};

#endif // RUDDERMOTOR_H
