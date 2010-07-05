/************************************************************************/
/*                                  									*/
/*		       P R O J E K T    A V A L O N                 			*/
/*								                                    	*/
/*	    ports.h	        The Ports-structure                             */
/*									                                    */
/*	    Author  	    Mario Krucker                                   */
/*                      kruckema@ethz.ch                        */
/*									                                    */
/************************************************************************/

#ifndef POTI_H
#define POTI_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(PotiData,
    struct
    {
       double sail_angle_abs;  //angle of the sail in degrees
    }
);
#endif // POTI_H
