/************************************************************************/
/*								                                    	*/
/*		                 P R O J E K T    A V A L O N          			*/
/*								                                    	*/
/*	  destination.h		The Class to store the ais Obstacles        	*/
/*								                                    	*/
/*	            Last Change	28. July 2009; Gion-Andri Büsser        	*/
/*							                                    		*/
/************************************************************************/

#ifndef AISEVAL_H
#define AISEVAL_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(ShipStruct,
		struct {
     
        unsigned long mmsi;
        double timestamp;

        double longitude; //everything in meters, middle point of (expected impact)
        double latitude;

        }
);

DDX_STORE_TYPE(ShipData,
        struct {

        ShipStruct Data[15];
        int shipCount;     //states how many ships have to be encountered (max 15)

        }

);

#endif //aisEval.h
