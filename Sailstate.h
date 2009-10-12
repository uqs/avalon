/************************************************************************/
/*																		*/
/*					P R O J E K T    A V A L O N						*/
/*																		*/
/*			Shipstate.h     State of the Sail                           */	
/*																		*/
/*			Author  		Stefan Wismer				                */
/*                          wismerst@student.ethz.ch                    */
/*																		*/
/************************************************************************/

#ifndef SAIL_STATE_H
#define SAIL_STATE_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(Sailstate,
  struct {
	float degrees_sail;				// Encoder Value of the Sail (degrees)

    float ref_sail;                 // Refernece Value of the sail
}
);

#endif // SAIL_STATE_H
