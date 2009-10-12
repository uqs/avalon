/************************************************************************/
/*								                                    	*/
/*		                 P R O J E K T    A V A L O N          			*/
/*								                                    	*/
/*	         lakenavigation.h		The Class to store navi-Infos   	*/
/*								                                    	*/
/*	            Last Change	14.April 2009; Gion-Andri Büsser        	*/
/*							                                    		*/
/************************************************************************/

#ifndef LAKENAVIGATION_H
#define LAKENAVIGATION_H

#include <DDXStore.h>
#include <DDXVariable.h>


#define AV_WYP_TYPE_BUOY    2
#define AV_WYP_TYPE_END     1
#define AV_WYP_TYPE_PASSBY  0


DDX_STORE_TYPE(WaypointStruct,
		struct {
        
        //heading to the following coordinates:
        //
        double heading; //in degrees
        
        //coordinates:

        int longitude; //degrees
        int latitude; //degrees

        //info to the type of waypoint:
        
        int wyp_type;
        int passed;

        //general infos to the previous calculation:
        
        double windspeed; //knots
        double winddirection; //global winddirection (-180 to 180)

		}
);

DDX_STORE_TYPE(WaypointData,
		struct {

        WaypointStruct Data[100]; //to store the consecutive waypoints in an array

		}
);

#endif //lakenavigation.h
