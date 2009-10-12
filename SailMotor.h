/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N	                */
/*								                                    	*/
/*  	SailMotor.h 	  Class for the SailMotor                       */
/*									                                    */
/*	    Author  	      Stefan Wismer			                        */
/*                        wismerst@student.ethz.ch                      */
/*									                                    */
/************************************************************************/

#ifndef SAILMOTOR_H
#define SAILMOTOR_H

class SailMotor {

public:
	SailMotor();
	~SailMotor();

	bool init(const char* device);

	bool calibrate(int id);				// Sets the current position to  move to

	bool move_to_angle(float degrees, float reference, int& feedback, int& num_rounds);		// Move the Sail

	bool emergency_stop();

    int sign(int i);
    int sign(float i);
    int sign(double i);
	

protected:
	// Maybe sometime there will be something here...

};

#endif // SAILMOTOR_H
