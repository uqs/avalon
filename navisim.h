/************************************************************************/
/*								                                    	*/
/*		                 P R O J E K T    A V A L O N          			*/
/*								                                    	*/
/*	  destination.h		The Class to store the end_destination        	*/
/*								                                    	*/
/*	            Last Change	14.April 2009; Gion-Andri Büsser        	*/
/*							                                    		*/
/************************************************************************/

#ifndef NAVISIM_H
#define NAVISIM_H

#include <DDXStore.h>
#include <DDXVariable.h>


DDX_STORE_TYPE(SimData,
		struct {
      
        double start_longitude; //everything in meters, already transformed by dest_transf
        double start_latitude;

        double end_longitude; //everything in meters, already transformed by dest_transf
        double end_latitude;

        int successfull;         //if 0 -> not yet passed, if 1 already passed!
        int count; 
        
        }
);


#endif //destination.h
