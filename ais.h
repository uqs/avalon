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

DDX_STORE_TYPE(Obstacle,
        struct {

        double angle;
        double dist;		// distance from current position to obstacle
	double t_crit;
	
	double longitude;	// Position of the obstacle
	double latitude;

        }

);

DDX_STORE_TYPE(AisDestData,
        struct {

        double new_dest_long;
	double new_dest_lat;

	unsigned int ais_dest_index;
	int global_skipper_flag;

        }

);

#endif //AIS_H
