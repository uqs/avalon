/************************************************************************/
/*                                  									*/
/*		       P R O J E K T    A V A L O N                 			*/
/*								                                    	*/
/*								                                    	*/
/*	    sail-target.h	    Where the sail shall be driven to		    */
/*									                                    */
/*	    Author          	Stefan Wismer			                    */
/*                          wismerst@student.ethz.ch                    */
/*									                                    */
/************************************************************************/

#ifndef SAIL_TARGET_H
#define SAIL_TARGET_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(sailTarget,
  struct {
    float degrees;		// Where to put the sail?

    int reset_request;  // 1 if a reset was requested.
}
);

#endif // SAIL_TARGET_H
