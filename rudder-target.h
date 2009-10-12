/************************************************************************/
/*                                  									*/
/*		       P R O J E K T    A V A L O N                 			*/
/*								                                    	*/
/*								                                    	*/
/*	    rudder-target.h	    Where the rudder shall be driven to		    */
/*									                                    */
/*	    Author          	Stefan Wismer			                    */
/*                          wismerst@student.ethz.ch                    */
/*									                                    */
/************************************************************************/

#ifndef RUDDER_TARGET_H
#define RUDDER_TARGET_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(rudderTarget,
    struct
    {
        double degrees_left;	    // Where to put the port rudder
        double degrees_right;	    // Where to put the starboard rudder

        int resetleft_request;      // 1 if a reset of the port rudder was requested
        int resetright_request;     // 1 if a reset of the starboard rudder was requested
    }
);
#endif // RUDDER_TARGET_H
