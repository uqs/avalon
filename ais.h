/************************************************************************/
/*									*/
/*		       P R O J E K T    C A S T O R 			*/
/*								 	*/
/*	ais.h		die Windsensor-Klasse				*/
/*									*/
/*	Last Change	26.Februar 2009; Patrick Schwizer		*/
/*									*/
/************************************************************************/

#ifndef AIS_H
#define AIS_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(AisStruct,
        struct {

        unsigned long mmsi;
        unsigned int navigational_status; 	//0= under way; 1=at anchor;2=not under command; 3=restricted manoeuvrability
        double rate_of_turn;	//degrees per minute
        unsigned int speed_over_ground;	//in knots
        unsigned int position_accuracy;	//1= high (<10m), 0=low (>10m)
        double longitude; 	//east=positive, west=negative
        double latitude;	 	//nort=positive, south=negative
        double course_over_ground;//in °
        double heading;		//in ° (external sensor); 511 indicates not available
        char destination[21];
        double time_of_arrival;	//mmddhhmm: month, day, hour, minute

        double timestamp;

        }
);

DDX_STORE_TYPE(AisData,
        struct {

        int number_of_ships; //how many ships are in sight?

        AisStruct Ship[15];        

        }
);



#endif //AIS_H
