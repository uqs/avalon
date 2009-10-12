/************************************************************************/
/*                                  									*/
/*		       P R O J E K T    A V A L O N                 			*/
/*								                                    	*/
/*	    ports.h	        The Ports-structure                             */
/*									                                    */
/*	    Author  	    Stefan Wismer                                   */
/*                      wismerst@student.ethz.ch                        */
/*									                                    */
/************************************************************************/

#ifndef PORTS_H
#define PORTS_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(Ports,
    struct
    {
       int rudderleft;              // Number of ports.
       int rudderright;             // /dev/ttyUSB[x]
       int sail;
    }
);
#endif // PORTS_H
