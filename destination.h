/************************************************************************/
/*								                                    	*/
/*		                 P R O J E K T    A V A L O N          			*/
/*								                                    	*/
/*	  destination.h		The Class to store the end_destination        	*/
/*								                                    	*/
/*	            Last Change	14.April 2009; Gion-Andri Büsser        	*/
/*							                                    		*/
/************************************************************************/

#ifndef DESTINATION_H
#define DESTINATION_H

#include <DDXStore.h>
#include <DDXVariable.h>

#define AV_DEST_TYPE_END  12
#define AV_DEST_TYPE_BUOY 11
#define AV_DEST_TYPE_NOMORE 13
#define AV_DEST_TYPE_OCEANWYP 14

DDX_STORE_TYPE(DestinationStruct,
		struct {
      
        double longitude; //everything in GPS-Coordinates, already transformed by dest_transf
        double latitude;

        int passed;         //if 0 -> not yet passed, if 1 already passed!
        int type; 
        
        }
);

DDX_STORE_TYPE(DestinationData,
        struct {

        DestinationStruct Data[1000];
        double longitude;               //the current final dest for the navigator in meters
        double latitude; 

        int destNr;
	int destNr_total;
  
	int not_in_list;
	unsigned int skipper_index_call;

        }

);

#endif //destination.h
