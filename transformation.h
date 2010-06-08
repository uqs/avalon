/************************************************************************/
/*								                                    	*/
/*		                 P R O J E K T    A V A L O N          			*/
/*								                                    	*/
/*	  transformation.h		The Class to store Transformation-Details  	*/
/*								                                    	*/
/*	            Last Change	14.April 2009; Gion-Andri Büsser        	*/
/*							                                    		*/
/************************************************************************/

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(LakeTransformation,
		struct {
       
        double longitude_start; //in degrees
        double latitude_start;  //in degrees
        double x_start_transf; 	//in meters
        double y_start_transf;  //in meters

        //endpoint
        double longitude_end;   //in degrees
        double latitude_end;    //in degrees
        double x_end_transf;  	//in meters
        double y_end_transf;    //in meters

        // offsets so we dont have to start at 0/0 ...
        int x_iter_offset;
        int y_iter_offset;

        //length in map-coordinates:
        int x_length;
        int y_length;

        //start and end nodes in map-coordinates:

        int x_start;
        int y_start;
        int x_end;
        int y_end;
        
        // real position offset, to afterwards calculate the real waypoint
        // positions
        int x_offset;
        int y_offset;

		}
);


#endif //coordination transformation .h
