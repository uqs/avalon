/************************************************************************/
/*																		*/
/*					P R O J E K T    A V A L O N						*/
/*																		*/
/*			Rudderstate.h   Current state of the rudder         		*/
/*																		*/
/*			Author  		Stefan Wismer				                */
/*																		*/
/************************************************************************/

#ifndef RUDDER_STATE_H
#define RUDDER_STATE_H

#include <DDXStore.h>
#include <DDXVariable.h>


DDX_STORE_TYPE(Rudderstate,
  struct {
	float degrees_rudder;		// Encoder Value of the Rudders  (degrees)
    float ref_rudder;          // Zero-Points for the Rudders ...
  }
);

#endif // RUDDER_STATE_H
