/************************************************************************/
/*									*/
/*		       P R O J E K T    A V A L O N 			*/
/*								 	*/
/*	rudder-target.h	Where the rudder shall be driven to		*/
/*									*/
/*	Last Change	06. March 2009; Stefan Wismer			*/
/*									*/
/************************************************************************/

#ifndef RUDDER_TARGET_H
#define RUDDER_TARGET_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(rudderTarget,
  struct {
	double degrees1;	// Where to put the port rudder
	double degrees2;	// Where to put the starboard rudder
	int valid;		// Is this valid (true) or just some random number (false)?
}
);

#endif // RUDDER_TARGET_H
