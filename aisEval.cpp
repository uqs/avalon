/**
 * Skipper calls the navigations-programs and sets a current heading to the
 * store, so sailor can take over!!
 *
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>


// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/time.h>
#include <rtx/thread.h>
#include <rtx/message.h>

// Specific Things
#include "include/ddxjoystick.h"

#include <DDXStore.h>
#include <DDXVariable.h>

#include "imu.h"
#include "aisEval.h"
#include "ais.h"
#include "destination.h"
#include "flags.h"
#include "imucleaner.h"

#include <vector>
#define DEBUG_AISEVAL

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataBoat; //to get imu-Data
DDXVariable dataImuClean;
DDXVariable shipData; //already prcessed information, showing the dangerous points of impact
DDXVariable shipStruct;
DDXVariable aisData; //actual ais data
DDXVariable aisStruct;
DDXVariable obstacle;
DDXVariable destinationData;
DDXVariable destinationStruct;
DDXVariable dataFlags;
DDXVariable skipperFlagData;
/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(double i);



/**
 * Storage for the command line arguments
 * */

const char * varname4 = "imu";
const char * varname_imuClean = "cleanimu";
const char * varname_shipData = "shipData";
const char * varname_shipStruct = "shipStruct";
const char * varname_aisData = "aisData";
const char * varname_aisStruct = "aisStruct";
const char * varname_Obstacle = "Obstacle";
const char * varname_destData = "destData";
const char * varname_destStruct = "destStruct";
const char * varname_flags = "flags";
const char * varname_skipperflags = "skipperflags";
const char * producerHelpStr = "skipper help-string";

/**
 * Command line arguments   //has yet to be completed
 *
 * */
RtxGetopt producerOpts[] = {

  {"imuData", "Store Variable where the imuData is written",
   {
     {RTX_GETOPT_STR, &varname4, "imuData"},
     RTX_GETOPT_END_ARG
   }
  },

  {"cleanimuname", "Store Variable where the cleaned imu data is written to",
   {
     {RTX_GETOPT_STR, &varname_imuClean, ""},
     RTX_GETOPT_END_ARG
   }
  },

  {"destination","Variable where the calculated navi-waypoints are stored",
   {
     {RTX_GETOPT_STR, &varname_destData, "Waypoints"},
     RTX_GETOPT_END_ARG
   }
  },
#if 0
  {"weatherData", "Store Variable where the weather data is written",
   {
     {RTX_GETOPT_STR, &varname_weather, ""},
     RTX_GETOPT_END_ARG
   }
  },
#endif
  RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{

    imuData boatData; //avalon's imu data
    imuCleanData imu_clean;
    ShipData ship; //transformed information about ship (ais)
    AisData ais;				//gps-struct
    Obstacle obst;
    DestinationData destination; 
    Flags generalflags;
    SkipperFlags skipperflags;	

    double current_pos_longitude, current_pos_latitude; //already transformed and in meters
    int p, k, i, n, interchange;
    unsigned int l;
    double ship_pos_longitude, ship_pos_latitude;
//     double ship_pos_longitude_dynamic, ship_pos_latitude_dynamic;
    RtxTime time_now;
    double time_now_d;
    double distance_static;// distance_dynamic;
    // int simtime;
    double heading_ship;
    bool inserted;
    
    double heading_avalon;
    double speed_avalon;
    double vel_avalon_long;
    double vel_avalon_lat;
    double speed_ship;
    double vel_ship_long;
    double vel_ship_lat;
    double vel_relativ_long;
    double vel_relativ_lat;
    double angle_relativ;
    double angle_avalon_ship;
    double threshold_radius;
    double radius_relativ;
    double dist_avalon_ship;
    double angle_tang_rear;
    double angle_tang_front;
    double angle_crit_min;
    double angle_crit_max;
    double safe_waypoint_long;
    double safe_waypoint_lat;
    double ship_length;
    double check_angle;
    double dist_min;
    double dist_max;
    double dist_limit;
    double dist_dest;
    double dist_test;
    double add_dist_safe=1000;

    std::vector<Obstacle> obst_p;
    std::vector<Obstacle> obst_p_start;
    std::vector<Obstacle> obst_p_end;
    std::vector<double> speed_avalon_all;
    Obstacle temp_obst;
    unsigned int num_speed_history = 20;
    int num_obstP;
    
    
    

    obst_p.reserve(4);

    while (1)
    {
 
        dataFlags.t_readto(generalflags,0,0);

        // Read the next data available, or wai
        if (dataBoat.t_readto(boatData,10,1) && (generalflags.state == AV_FLAGS_ST_NORMALSAILING || generalflags.state == AV_FLAGS_ST_UPWINDSAILING || generalflags.state == AV_FLAGS_ST_DOWNWINDSAILING ))
        {
            aisData.t_readto(ais,0,0);
	    obstacle.t_readto(obst,0,0);
            shipData.t_readto(ship,0,0);
            destinationData.t_readto(destination,0,0);
            dataImuClean.t_readto(imu_clean,0,0);
            

            current_pos_longitude = (AV_EARTHRADIUS 
                    *cos((boatData.position.latitude * AV_PI/180))*(AV_PI/180)
                    *boatData.position.longitude);
            current_pos_latitude = (AV_EARTHRADIUS
                    *(AV_PI/180)*boatData.position.latitude);



            //clean up our list of dangerous ships:
#ifdef DEBUG_AISEVAL
            rtx_message("while loop going through, shipCount = %d, aisCount = %d \n",ship.shipCount, ais.number_of_ships);
#endif

            for( p = 0; p < ship.shipCount; p++)
            {
#ifdef DEBUG_AISEVAL
                rtx_message("checking if the ship is already in shipData( %d times)\n",p);
#endif
                rtx_time_get(&time_now); 
                time_now_d = rtx_time_to_double(&time_now);
                if ((time_now_d - ship.Data[p].timestamp) > 1200)
                {
                    ship.Data[p].mmsi = ship.Data[ship.shipCount].mmsi;
                    ship.Data[p].timestamp = ship.Data[ship.shipCount].timestamp;
                    ship.Data[p].longitude = ship.Data[ship.shipCount].longitude;
                    ship.Data[p].latitude = ship.Data[ship.shipCount].latitude;

                    ship.shipCount --;
                }
            }



	    for (i = 0; i < ais.number_of_ships; i++)
            {
#ifdef DEBUG_AISEVAL
                rtx_message("checking for ships %d. times\n",p);
#endif
                ship_pos_longitude = (AV_EARTHRADIUS 
                        *cos((ais.Ship[i].latitude * AV_PI/180))*(AV_PI/180)
                        *ais.Ship[i].longitude);
                ship_pos_latitude = (AV_EARTHRADIUS
                        *(AV_PI/180)*ais.Ship[i].latitude);

                distance_static = sqrt((current_pos_longitude - ship_pos_longitude)
                        *(current_pos_longitude - ship_pos_longitude)
                        + (current_pos_latitude - ship_pos_latitude)
                        *(current_pos_latitude - ship_pos_latitude));

// ASSUMPTION:  ONLY TAKE THE CLOSEST SHIP! -> bubble_sort for example....

#ifdef DEBUG_AISEVAL
                rtx_message("static distance avalon to ship is %f meters\n",distance_static);
                rtx_message("ship's longitude = %f \n",ship_pos_longitude);
                rtx_message("ship's latitude = %f \n",ship_pos_latitude);
#endif
                if ((distance_static < 20000.0) && (ais.Ship[i].speed_over_ground != 0.0)
                        && ((ais.Ship[i].course_over_ground != 0.0) || (ais.Ship[i].heading != 0.0)))
                {
		    heading_ship = ais.Ship[i].heading*AV_PI/180.0; // TODO degree or rad??
                    if (heading_ship == 0.0)
                    {
                        heading_ship = ais.Ship[i].course_over_ground*AV_PI/180.0;
                    }
#ifdef DEBUG_AISEVAL
                    rtx_message("ship in reachable distance (20 km) \n");
#endif

		    speed_ship		= 0.5144444 * ais.Ship[i].speed_over_ground;
                    heading_avalon     	= atan2((destination.latitude - current_pos_latitude) , (destination.longitude - current_pos_longitude));
		    speed_avalon	= sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));
//                     speed_avalon_all.insert(speed_avalon_all.begin(),speed_avalon);
// 		    if(speed_avalon_all.size()>num_speed_history)
// 		      {speed_avalon_all.resize(num_speed_history);}
// 		    speed_avalon = 0;
// 		    for (l=0;l<speed_avalon_all.size();l++)
// 		      {speed_avalon = speed_avalon + 1.0/speed_avalon_all.size()*speed_avalon_all[l];}

                    vel_avalon_long	= sin(heading_avalon) * speed_avalon;
                    vel_avalon_lat	= cos(heading_avalon) * speed_avalon;
                    vel_ship_long	= sin(heading_ship) * speed_ship;
                    vel_ship_lat	= cos(heading_ship) * speed_ship;
                    vel_relativ_long	= vel_avalon_long - vel_ship_long;
                    vel_relativ_lat	= vel_avalon_lat - vel_ship_lat;

		    angle_relativ	= atan2(vel_relativ_lat, vel_relativ_long);
                    angle_avalon_ship	= atan2((ship_pos_longitude-current_pos_longitude) , (ship_pos_latitude-current_pos_latitude));

                    ship_length		= 100;  // ship length, if not from AIS
                    threshold_radius	= 500;  // to be modified: additional safety distance
                    radius_relativ	= ship_length + 4 + threshold_radius;    // ship length + avalon_length + threshold_radius
                    dist_avalon_ship	= sqrt(((ship_pos_latitude-current_pos_latitude)*(ship_pos_latitude-current_pos_latitude))
					      +((ship_pos_longitude-current_pos_longitude)*(ship_pos_longitude-current_pos_longitude)));

                    angle_tang_rear	= remainder(((angle_avalon_ship - asin(radius_relativ/dist_avalon_ship))*180.0/AV_PI),360.0) * AV_PI/180.0;
                    angle_tang_front	= remainder(((angle_avalon_ship + asin(radius_relativ/dist_avalon_ship))*180.0/AV_PI),360.0) * AV_PI/180.0;
                    
		    if ((remainder(((angle_relativ - angle_tang_rear)*180.0/AV_PI),360.0)>0)
		      && (remainder(((angle_relativ - angle_tang_front)*180.0/AV_PI),360.0)<0))
		    {
			// On collision course with a ship, check ALL ships for obstacles
			skipperflags.global_locator  = AV_FLAGS_GLOBALSK_COLLISION;
			skipperFlagData.t_writefrom(skipperflags);
			break;
		    }
		}
	    }

	    if (skipperflags.global_locator != AV_FLAGS_GLOBALSK_COLLISION)
		{continue;}

	    obst_p_end.clear();
	    obst_p_start.clear();
	    // find the collision points with every ship in the vicinity
	    for (i = 0; i < ais.number_of_ships; i++)
	    {
#ifdef DEBUG_AISEVAL
		rtx_message("checking for ships %d. times\n",p);
#endif
		ship_pos_longitude = (AV_EARTHRADIUS 
			*cos((ais.Ship[i].latitude * AV_PI/180))*(AV_PI/180)
			*ais.Ship[i].longitude);
		ship_pos_latitude = (AV_EARTHRADIUS
			*(AV_PI/180)*ais.Ship[i].latitude);

		distance_static = sqrt((current_pos_longitude - ship_pos_longitude)
			*(current_pos_longitude - ship_pos_longitude)
			+ (current_pos_latitude - ship_pos_latitude)
			*(current_pos_latitude - ship_pos_latitude));


#ifdef DEBUG_AISEVAL
                rtx_message("static distance avalon to ship is %f meters\n",distance_static);
                rtx_message("ship's longitude = %f \n",ship_pos_longitude);
                rtx_message("ship's latitude = %f \n",ship_pos_latitude);
#endif
		if ((distance_static < 20000.0) && (ais.Ship[i].speed_over_ground != 0.0)
			&& ((ais.Ship[i].course_over_ground != 0.0) || (ais.Ship[i].heading != 0.0)))
		{
		    obst_p.clear();
		    num_obstP=0;
		    heading_ship = ais.Ship[i].heading*AV_PI/180.0;	// TODO degree or rad??
		    if (heading_ship == 0.0)
		    {
			heading_ship = ais.Ship[i].course_over_ground*AV_PI/180.0;
		    }
#ifdef DEBUG_AISEVAL
                    rtx_message("ship in reachable distance (20 km) \n");
#endif

		    speed_ship		= 0.5144444 * ais.Ship[i].speed_over_ground;
		    heading_avalon     	= atan2((destination.latitude - current_pos_latitude) , (destination.longitude - current_pos_longitude));
		    speed_avalon       	= sqrt((imu_clean.velocity.x*imu_clean.velocity.x) + (imu_clean.velocity.y*imu_clean.velocity.y));

    // 		speed_avalon_all.insert(speed_avalon_all.begin(),speed_avalon);
    // 		if(speed_avalon_all.size()>num_speed_history)
    // 		    {speed_avalon_all.resize(num_speed_history);}
    // 		speed_avalon = 0;
    // 		for (l=0;l<speed_avalon_all.size();l++)
    // 		    {speed_avalon = speed_avalon + 1.0/speed_avalon_all.size()*speed_avalon_all[l];}

		    vel_avalon_long	= sin(heading_avalon) * speed_avalon;
		    vel_avalon_lat	= cos(heading_avalon) * speed_avalon;
		    vel_ship_long	= sin(heading_ship) * speed_ship;
		    vel_ship_lat	= cos(heading_ship) * speed_ship;
		    vel_relativ_long	= vel_avalon_long - vel_ship_long;
		    vel_relativ_lat	= vel_avalon_lat - vel_ship_lat;

		    angle_relativ	= atan2(vel_relativ_lat, vel_relativ_long);
		    angle_avalon_ship	= atan2((ship_pos_longitude-current_pos_longitude) , (ship_pos_latitude-current_pos_latitude));

		    ship_length		= 100;  // ship length, if not from AIS
		    threshold_radius	= 500;  // to be modified: additional safety distance
		    radius_relativ		= ship_length + 4 + 500;    // ship length + avalon_length + threshold_radius
		    dist_avalon_ship	= sqrt(((ship_pos_latitude-current_pos_latitude)*(ship_pos_latitude-current_pos_latitude))
						  +((ship_pos_longitude-current_pos_longitude)*(ship_pos_longitude-current_pos_longitude)));

		    angle_tang_rear	= remainder(((angle_avalon_ship - asin(radius_relativ/dist_avalon_ship))*180.0/AV_PI),360.0) * AV_PI/180.0;
		    angle_tang_front	= remainder(((angle_avalon_ship + asin(radius_relativ/dist_avalon_ship))*180.0/AV_PI),360.0) * AV_PI/180.0;
		    


		    if (speed_avalon < speed_ship)
		    {
			angle_crit_min = remainder(((AV_PI+heading_ship - asin(speed_avalon/speed_ship))*180.0/AV_PI),360.0) * AV_PI/180.0;
			angle_crit_max = remainder(((AV_PI+heading_ship + asin(speed_avalon/speed_ship))*180.0/AV_PI),360.0) * AV_PI/180.0;

			if ((remainder(((angle_crit_min-angle_tang_rear)*180.0/AV_PI),360.0) < 0)
			  && (remainder(((angle_crit_max-angle_tang_rear)*180.0/AV_PI),360.0) > 0))
			{	// Calculate the two intersectin with the first tangent
			    obst_p.resize(num_obstP+2);

			    obst_p[num_obstP].angle 	= remainder(((angle_tang_rear
							  -asin(speed_avalon/speed_ship*sin(AV_PI+heading_ship-angle_tang_rear)))*180.0/AV_PI),360.0) * AV_PI/180.0;
			    obst_p[num_obstP].t_crit	= ((current_pos_longitude-ship_pos_longitude)*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(current_pos_latitude-ship_pos_latitude)*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)))
							  /((vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)));
			    obst_p[num_obstP].dist	= speed_avalon * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].longitude	= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_longitude;
			    obst_p[num_obstP].latitude	= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_latitude;
			    num_obstP ++;
			    

			    obst_p[num_obstP].angle 	= remainder(((angle_tang_rear
							  +asin(speed_avalon/speed_ship*sin(AV_PI+heading_ship-angle_tang_rear))-AV_PI)*180.0/AV_PI),360.0) * AV_PI/180.0;
			    obst_p[num_obstP].t_crit	= ((current_pos_longitude-ship_pos_longitude)*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(current_pos_latitude-ship_pos_latitude)*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)))
							  /((vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)));
			    obst_p[num_obstP].dist	= speed_avalon * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].longitude	= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_longitude;
			    obst_p[num_obstP].latitude	= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_latitude;
			    num_obstP ++;
			}

			if ((remainder(((angle_crit_min-angle_tang_front)*180.0/AV_PI),360.0) < 0) && (remainder(((angle_crit_max-angle_tang_front)*180.0/AV_PI),360.0) > 0))
			{	// Calculate the two intersectin with the second tangent
			    obst_p.resize(num_obstP+2);

			    obst_p[num_obstP].angle 	= remainder(((angle_tang_front
							  -asin(speed_avalon/speed_ship*sin(AV_PI+heading_ship-angle_tang_front)))*180.0/AV_PI),360.0) * AV_PI/180.0;
			    obst_p[num_obstP].t_crit	= ((current_pos_longitude-ship_pos_longitude)*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(current_pos_latitude-ship_pos_latitude)*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)))
							  /((vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)));
			    obst_p[num_obstP].dist	= speed_avalon * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].longitude	= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_longitude;
			    obst_p[num_obstP].latitude	= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_latitude;
			    num_obstP ++;
			    

			    obst_p[num_obstP].angle 	= remainder(((angle_tang_front
							  +asin(speed_avalon/speed_ship*sin(AV_PI+heading_ship-angle_tang_front))-AV_PI)*180.0/AV_PI),360.0) * AV_PI/180.0;
			    obst_p[num_obstP].t_crit	= ((current_pos_longitude-ship_pos_longitude)*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle)) 
							  +(current_pos_latitude-ship_pos_latitude)*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)))
							  /((vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)));
			    obst_p[num_obstP].dist	= speed_avalon * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].longitude	= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_longitude;
			    obst_p[num_obstP].latitude	= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_latitude;
			    num_obstP ++;
			}
		    }
		    else	// if speed_avalon > speed_ship, we have an intersection with each tangent
		    {
			obst_p.resize(num_obstP+2);

			obst_p[num_obstP].angle 	= remainder(((angle_tang_rear 
							  -asin(speed_avalon/speed_ship*sin(AV_PI+heading_ship-angle_tang_rear)))*180.0/AV_PI),360.0) * AV_PI/180.0;
			obst_p[num_obstP].t_crit	= ((current_pos_longitude-ship_pos_longitude)*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle)) 
							  +(current_pos_latitude-ship_pos_latitude)*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))) 
							  /((vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle)) 
							  +(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)));
			obst_p[num_obstP].dist		= speed_avalon * obst_p[num_obstP].t_crit + add_dist_safe;
			obst_p[num_obstP].longitude	= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_longitude;
			obst_p[num_obstP].latitude	= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_latitude;
			num_obstP ++;
			  
			
			obst_p[num_obstP].angle 	= remainder(((angle_tang_front 
							  -asin(speed_avalon/speed_ship*sin(AV_PI+heading_ship-angle_tang_front))-AV_PI)*180.0/AV_PI),360.0) * AV_PI/180.0;
			obst_p[num_obstP].t_crit	= ((current_pos_longitude-ship_pos_longitude)*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(current_pos_latitude-ship_pos_latitude)*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))) 
							  /((vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))*(vel_ship_long - vel_avalon_long*sin(obst_p[num_obstP].angle))
							  +(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle))*(vel_ship_lat - vel_avalon_lat*cos(obst_p[num_obstP].angle)));
			obst_p[num_obstP].dist		= speed_avalon * obst_p[num_obstP].t_crit + add_dist_safe;
			obst_p[num_obstP].longitude	= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_longitude;
			obst_p[num_obstP].latitude	= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_latitude;
			num_obstP ++;
		    }


		    // sort obstacles upon their angles with bubble sort
		    n=num_obstP;
		    while (interchange)
		    {
			interchange=0;
			for( p = 0; p < n-1; p++)
			{
			    if(obst_p[p].angle > obst_p[p+1].angle)
			    {
				temp_obst=obst_p[p];
				obst_p[p]=obst_p[p+1];
				obst_p[p+1]=temp_obst;
				interchange=1;
			    }
			}
			n--;
		    }

		    // define if collision points are starts or ends of an obstacles
		    if(num_obstP>0)
		    {
			check_angle = atan2(speed_avalon*sin((obst_p[0].angle + obst_p[1].angle)/2)-vel_ship_long,
					    speed_avalon*cos((obst_p[0].angle + obst_p[1].angle)/2)-vel_ship_lat);
			if ((remainder(((check_angle - angle_tang_rear)*180.0/AV_PI),360.0)<=0) 
			  || (remainder(((check_angle - angle_tang_front)*180.0/AV_PI),360.0)>=0))
			{
			    temp_obst=obst_p[0];
			    for ( p = 0; p < num_obstP-1; p++)
			    {
			      obst_p[p]=obst_p[p+1];
			    }
			    obst_p[num_obstP-1]=temp_obst;
			}
		      
			// if an obstacle is too far away, we don't care about it
			dist_dest =(sqrt((current_pos_longitude - destination.longitude)
					*(current_pos_longitude - destination.longitude)
					+(current_pos_latitude - destination.latitude)
					*(current_pos_latitude - destination.latitude)));
			dist_limit = 2*dist_dest;
			for ( p = 0; p < num_obstP-1; p=p+2)
			{
			    if ((obst_p[p].dist<dist_max) || (obst_p[p+1].dist<dist_max))
			    {
				obst_p_start.push_back(obst_p[p]);
				obst_p_end.push_back(obst_p[p+1]);
			    }
			}
		    }
		}
	    }
		  
	    // Fuse obstacles
	    n=obst_p_start.size()-1;
	    k=0;
	    while (n>=k)
	    {
		p=0;
		while (n>=p)
		{
		    if (obst_p_start[k].angle<obst_p_start[p].angle && obst_p_end[k].angle>obst_p_start[p].angle)
		    {
			if (obst_p_end[k].angle<obst_p_end[p].angle)
			{
			    obst_p_end[k].angle=obst_p_end[p].angle;
			}
			obst_p_start.erase(obst_p_start.begin()+p);
			obst_p_end.erase(obst_p_end.begin()+p);
			n--;
		    }
		    p++;
		}
		k++;
	    }
		  
	    // set new Waypoint
	    safe_waypoint_long	= obst_p_start[0].longitude; 
	    safe_waypoint_lat	= obst_p_start[0].latitude;
	    dist_min		= obst_p_start[0].dist + sqrt((obst_p_start[0].longitude-destination.longitude)
							*(obst_p_start[0].longitude-destination.longitude)
							+(obst_p_start[0].latitude-destination.latitude)
							*(obst_p_start[0].latitude-destination.latitude));
	    for (l=1;l<obst_p_start.size();l++)
	    {
		dist_test 	= obst_p_start[l].dist + sqrt((obst_p_start[l].longitude-destination.longitude)
							*(obst_p_start[l].longitude-destination.longitude)
							+(obst_p_start[l].latitude-destination.latitude)
							*(obst_p_start[l].latitude-destination.latitude));
		if(dist_min>dist_test)
		{
		    dist_min=dist_test;
		    safe_waypoint_long	= obst_p_start[l].longitude; 
		    safe_waypoint_lat	= obst_p_start[l].latitude;
		}
	    }
	    for (l=0;l<obst_p_end.size();l++)
	    {
		dist_test  	= obst_p_end[l].dist + sqrt((obst_p_end[l].longitude-destination.longitude)
						      *(obst_p_end[l].longitude-destination.longitude)
						      +(obst_p_end[l].latitude-destination.latitude)
						      *(obst_p_end[l].latitude-destination.latitude));
		if(dist_min>dist_test)
		{
		    dist_min=dist_test;
		    safe_waypoint_long	= obst_p_end[l].longitude; 
		    safe_waypoint_lat	= obst_p_end[l].latitude;
		}
	    }

	    destination.longitude = safe_waypoint_long;
	    destination.latitude  = safe_waypoint_lat;
	    destinationData.t_writefrom(destination);
	}
    }


                   
#if 0
                    for (simtime = 0; simtime < 40; simtime ++) //time in half-minutes...
                    {
#ifdef DEBUG_AISEVAL
                        printf("going through times (simtime = %d) \n",simtime);
#endif
                        ship_pos_longitude_dynamic += 
                            cos(remainder(((-(heading_ship - 90))*(AV_PI / 180)),2*AV_PI))
                            * simtime * 30 * 0.5144444 * ais.Ship[i].speed_over_ground;
                        ship_pos_latitude_dynamic +=
                            sin(remainder(((-(heading_ship - 90))*(AV_PI / 180)),2*AV_PI))
                            * simtime * 30 * 0.5144444 * ais.Ship[i].speed_over_ground;

                        distance_dynamic = sqrt((current_pos_longitude - ship_pos_longitude_dynamic)
                                *(current_pos_longitude - ship_pos_longitude_dynamic)
                                + (current_pos_latitude - ship_pos_latitude_dynamic)
                                *(current_pos_latitude - ship_pos_latitude_dynamic));

#ifdef DEBUG_AISEVAL
                        printf("dynamic_longitude = %f \n",ship_pos_longitude_dynamic);
                        printf("dynamic_latitude = %f \n",ship_pos_latitude_dynamic);
                        printf("dynamic_distance = %f \n",distance_dynamic);
#endif


                        if ((distance_dynamic / 1.800555) - (simtime * 30) < 120.0)
                        {
#ifdef DEBUG_AISEVAL
                            printf("potential crash point for simtime = %d detected\n",simtime);
#endif

                            inserted = false;
#endif
#if 0
                            for (q = 0; q < ship.shipCount; q++)
                            {
#ifdef DEBUG_AISEVAL
                                rtx_message("checking if ship already in the list (counter = %d) \n",q);
#endif
                                if (ais.Ship[i].mmsi == ship.Data[q].mmsi)
                                {
#ifdef DEBUG_AISEVAL
                                    rtx_message("updating an existing ship-entry\n");
#endif
                                    ship.Data[q].timestamp = ais.Ship[i].timestamp;
                                    ship.Data[q].longitude = ship_pos_longitude_dynamic;
                                    ship.Data[q].latitude = ship_pos_latitude_dynamic;
                                    inserted = true;
                                    break;
                                }
                            }
                            if (!inserted)
                            {
#if 0 
                                rtx_message("inserting a new ship at end of the list\n");
#endif
                                ship.Data[ship.shipCount].mmsi = ais.Ship[i].mmsi;
                                ship.Data[ship.shipCount].timestamp = ais.Ship[i].timestamp;
                                ship.Data[ship.shipCount].longitude = ship_pos_longitude_dynamic;
                                ship.Data[ship.shipCount].latitude = ship_pos_latitude_dynamic;
                                ship.shipCount ++;
                                inserted = true;
#ifdef DEBUG_AISEVAL
                                rtx_message("new shipCount = %d \n", ship.shipCount);
#endif

                            }
                            assert(inserted);
                            break;
            }        
            shipData.t_writefrom(ship);

        }
#endif
#if 0
        else if (dataWindClean.hasTimedOut()) {
			// Timeout. Probably no joystick connected.

			rtx_message("Timeout while reading dataWindClean \n");}

		else if (dataBoat.hasTimedOut()) {
			// Timeout. Probably no joystick connected.

			rtx_message("Timeout while reading IMU-Data \n");}

		else
		{
			// Something strange happend. Critical Error.
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}
        rtx_timer_sleep(0.1);
	}

	return NULL;

}
#endif
}

// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}

// Some self-defined utility functions:
int sign(int i) // gives back the sign of an int
{
    if (i>=0)
        return 1;
    else
        return -1;
}

int sign(double i) // gives back the sign of a float
{
    if (i>=0)
        return 1;
    else
        return -1;
}


int main (int argc, const char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("AIS Eval Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ShipStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ShipData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Obstacle));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
        DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));
        DOC(DDX_STORE_REGISTER_TYPE (store.getId(), SkipperFlags));
    //
	
    // Create output variable
        DOB(store.registerVariable(aisStruct, varname_aisStruct, "AisStruct"));
        DOB(store.registerVariable(aisData, varname_aisData, "AisData"));
	DOB(store.registerVariable(aisData, varname_Obstacle, "Obstacle"));
        DOB(store.registerVariable(dataBoat, varname4, "imuData"));
        DOB(store.registerVariable(shipStruct, varname_shipStruct, "ShipStruct"));
        DOB(store.registerVariable(shipData, varname_shipData, "ShipData"));
       DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));
        DOB(store.registerVariable(skipperFlagData, varname_skipperflags, "SkipperFlags"));
	
	// Start the working thread
    DOP(th = rtx_thread_create ("aisEval thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down aisEval...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);

}
