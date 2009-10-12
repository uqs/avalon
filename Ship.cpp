/************************************************************************/
/*									                                    */
/*		       P R O J E K T    A V A L O N 			                */
/*								 	                                    */
/*	Ship.cpp Class Fuctions for the Ship Class	                        */
/*									                                    */
/*	Last Change	March 19, 2009; Hendrik Erckens	                		*/
/*									                                    */
/************************************************************************/

#include <rtx/message.h>

#include "avalon.h"
#include "Ship.h"

/**
 * Class functions
 * */

Ship::Ship() // Constructor
{
    wanted_sail_angle_to_wind = 0;
}

Ship::~Ship(){} // Destructor
