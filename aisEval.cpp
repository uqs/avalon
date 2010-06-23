/**
 * Skipper calls the navigations-programs and sets a current heading to the
 * store, so sailor can take over!!
 *
 **/
// TODO check "remainder", speed history, which obstacles do we care about, 

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
// #define DEBUG_AISEVAL

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataBoat; //to get imu-Data
DDXVariable dataFlags;
DDXVariable dataImuClean;
DDXVariable shipData; //already prcessed information, showing the dangerous points of impact
DDXVariable shipStruct;
DDXVariable aisData; //actual ais data
DDXVariable aisStruct;
DDXVariable aisDestData;
DDXVariable destinationData;
DDXVariable destinationStruct;
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
const char * varname_aisDestData = "aisDestData";
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
#if 0
  {"destination","Variable where the calculated navi-waypoints are stored",
   {
     {RTX_GETOPT_STR, &varname_destData, "Waypoints"},
     RTX_GETOPT_END_ARG
   }
  },

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
    AisDestData ais_dest;
    DestinationData destination; 
    Flags generalflags;
    SkipperFlags skipperflags;	

    double current_pos_x, current_pos_y; //already transformed and in meters
    int p, k, i, n, interchange;
    unsigned int l;
    double ship_pos_x, ship_pos_y;
    double distance;
    double heading_ship;
    bool collision;
    
    double heading_avalon;
    double speed_avalon;
    double angle_to_dest;
    double speed_avalon_to_dest; 
    double vel_avalon_x, vel_avalon_y;
    double speed_ship;
    double vel_ship_x, vel_ship_y;
    double vel_relativ_x, vel_relativ_y;
    double angle_relativ;
    double angle_avalon_ship;
    double threshold_radius;
    double radius_relativ;
    double dist_avalon_ship;
    double angle_tang_rear, angle_tang_front;
    double angle_crit_min, angle_crit_max;
    double angle_safe;
    double safe_destpoint_x, safe_destpoint_y;
    double ship_length;
    double check_angle;
    double dist_min;
    double dist_limit;
    double dist_dest;
    double dist_test;
    double add_dist_safe=100, add_angle_safe=5*AV_PI/180.0;
    double dist_of_sight=20000; // in meter

    std::vector<Obstacle> obst_p;
    std::vector<Obstacle> obst_p_start;
    std::vector<Obstacle> obst_p_end;
    std::vector<double> speed_avalon_all;
    Obstacle temp_obst;
    unsigned int num_speed_history = 20;
    int num_obstP;
    
    
    dataFlags.t_readto(generalflags,0,0);
    ais_dest.new_dest_long=0;
    ais_dest.new_dest_lat=0;
    ais_dest.ais_dest_index=0;
    ais_dest.global_skipper_flag=generalflags.global_locator;
    aisDestData.t_writefrom(ais_dest);

    obst_p.reserve(4);

    while (1)
    {
        dataFlags.t_readto(generalflags,0,0);

        // Read the next data available, or wait at least 5 seconds
        if (dataBoat.t_readto(boatData,10,1) && (generalflags.state == AV_FLAGS_ST_NORMALSAILING || generalflags.state == AV_FLAGS_ST_UPWINDSAILING || generalflags.state == AV_FLAGS_ST_DOWNWINDSAILING ))
        {

            aisData.t_readto(ais,0,0);
            shipData.t_readto(ship,0,0);
            destinationData.t_readto(destination,0,0);
	    skipperFlagData.t_readto(skipperflags,0,0);
            dataImuClean.t_readto(imu_clean,0,0);

	    current_pos_x =AV_EARTHRADIUS * (AV_PI/180) * (boatData.position.latitude-destination.latitude);
	    current_pos_y =AV_EARTHRADIUS * cos((destination.latitude * AV_PI/180)) * (AV_PI/180)
			      *(boatData.position.longitude-destination.longitude);


#if 0
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
#endif

	    collision = false;
	    for (i = 0; i < ais.number_of_ships; i++)
            {
		ship_pos_x 	= AV_EARTHRADIUS * (AV_PI/180) * (ais.Ship[i].latitude-destination.latitude);
		ship_pos_y 	= AV_EARTHRADIUS * cos((destination.latitude * AV_PI/180)) * (AV_PI/180)
				    *(ais.Ship[i].longitude-destination.longitude);

		distance	= sqrt(pow(ship_pos_x - current_pos_x,2) + pow(ship_pos_y - current_pos_x,2));
#ifdef DEBUG_AISEVAL
                rtx_message("distance avalon to ship number %d is %f meters\n",i,distance);
                rtx_message("ship's position (x,y) = (%f,%f) \n",ship_pos_x, ship_pos_y);
#endif

                if ((distance < dist_of_sight) && (ais.Ship[i].speed_over_ground != 0.0)
                        && ((ais.Ship[i].course_over_ground != 0.0) || (ais.Ship[i].heading != 0.0)))
                {
		    heading_ship = ais.Ship[i].heading*AV_PI/180.0;
                    if (heading_ship == 0.0)
                    {
                        heading_ship = ais.Ship[i].course_over_ground*AV_PI/180.0;
                    }
#ifdef DEBUG_AISEVAL
                    rtx_message("ship in reachable distance (20 km) \n");
#endif

		    speed_ship		= 0.5144444 * ais.Ship[i].speed_over_ground;
                   
// rtx_message("avalon: x= %f y= %f, ship: x= %f y= %f  angle av-sh = %f \n", current_pos_x, current_pos_y, ship_pos_x,ship_pos_y, angle_avalon_ship*180/AV_PI);
		    // take the average velocity over the last "num_speed_history" measurements
		    speed_avalon	= 0.5144444 * sqrt(pow(imu_clean.velocity.x,2) + pow(imu_clean.velocity.y,2));
                    speed_avalon_all.insert(speed_avalon_all.begin(),speed_avalon);
		    if(speed_avalon_all.size()>num_speed_history)
		      {speed_avalon_all.resize(num_speed_history);}
		    speed_avalon = 0;
		    for (l=0;l<speed_avalon_all.size();l++)
		      {speed_avalon = speed_avalon + 1.0/speed_avalon_all.size()*speed_avalon_all[l];}

// 		    speed_avalon_to_dest = speed_avalon*cos(angle_to_dest-heading_avalon); // TODO what velocity and angle do we take?
		    speed_avalon_to_dest= speed_avalon;

		    heading_avalon	= boatData.attitude.yaw*AV_PI/180.0;
                    angle_avalon_ship	= atan2((ship_pos_y-current_pos_y) , (ship_pos_x-current_pos_x));
		    angle_to_dest	= atan2((-current_pos_y),(-current_pos_x));


// rtx_message("velocity avalon abs: %f vel to dest: %f \n",speed_avalon,speed_to_dest);

                    vel_avalon_x	= cos(angle_to_dest) * speed_avalon_to_dest;
                    vel_avalon_y	= sin(angle_to_dest) * speed_avalon_to_dest;
                    vel_ship_x		= cos(heading_ship) * speed_ship;
                    vel_ship_y		= sin(heading_ship) * speed_ship;
                    vel_relativ_x	= vel_avalon_x - vel_ship_x;
                    vel_relativ_y	= vel_avalon_y - vel_ship_y;
		    angle_relativ	= atan2(vel_relativ_y, vel_relativ_x);
// rtx_message("avalon vel:  long= %f  lat= %f  total= %f  angle= %f", vel_avalon_long,vel_avalon_lat,speed_avalon_to_dest, angle_to_dest);
// rtx_message("relativ vel: long= %lf lat= %lf", vel_avalon_long-vel_ship_long,vel_avalon_lat-vel_ship_lat);


                    ship_length		= 1;  // ship length, if not from AIS
                    threshold_radius	= 295;  // to be modified: additional safety distance
                    radius_relativ	= ship_length + 4 + threshold_radius;    // ship length + avalon_length + threshold_radius
                    dist_avalon_ship	= sqrt(pow(ship_pos_x - current_pos_x,2) + pow(ship_pos_y - current_pos_y,2));
// rtx_message("dist to ship = %f \n", dist_avalon_ship);
		    if (dist_avalon_ship<0.5*radius_relativ)
		    {
			if (remainder((AV_PI+angle_avalon_ship-heading_ship),2*AV_PI)<0)
			    {angle_safe = remainder(heading_ship-AV_PI*0.5,2*AV_PI);}
			else
			    {angle_safe = remainder(heading_ship+AV_PI*0.5,2*AV_PI);}

// rtx_message("angle_av_sh= %lf    head_sh= %lf     ang_safe= %lf\n", angle_avalon_ship*180/AV_PI,heading_ship*180/AV_PI,angle_safe*180/AV_PI);
			ais_dest.new_dest_long 	= destination.latitude + (ship_pos_x+1.5*radius_relativ*cos(angle_safe))*180/(AV_PI*AV_EARTHRADIUS);
			ais_dest.new_dest_lat 	= destination.longitude + (ship_pos_y+1.5*radius_relativ*sin(angle_safe))*180
									/ (AV_PI * AV_EARTHRADIUS * cos(destination.latitude*AV_PI/180));
			ais_dest.ais_dest_index ++;
			ais_dest.global_skipper_flag = AV_FLAGS_GLOBALSK_SURVIVE;
			aisDestData.t_writefrom(ais_dest);			

			continue;
		    }

		    angle_tang_rear	= remainder((angle_avalon_ship - asin(radius_relativ/dist_avalon_ship)),2*AV_PI);
		    angle_tang_front	= remainder((angle_avalon_ship + asin(radius_relativ/dist_avalon_ship)),2*AV_PI);
 rtx_message("ang_rear: %f  ang_front: %f  ang_rel: %f",angle_tang_rear*180/AV_PI,angle_tang_front*180/AV_PI, angle_relativ*180/AV_PI);
		    if ((remainder(((angle_relativ - angle_tang_rear)),2*AV_PI)>0)
		      && (remainder(((angle_relativ - angle_tang_front)),2*AV_PI)<0))
		    {
			// On collision course with a ship, check ALL ships for obstacles
			collision = true;
			break;
		    }
		}
	    }
// rtx_message("skipperflag: %d",generalflags.global_locator);
	    if (collision == false)
		{continue;}

#ifdef DEBUG_AISEVAL
		rtx_message("Collision course!! check for new destination\n");
#endif
rtx_message("Collision course!! check for new destination\n");
	    obst_p_end.clear();
	    obst_p_start.clear();

	    // find the collision points with every ship in the vicinity
	    for (i = 0; i < ais.number_of_ships; i++)
	    {
		ship_pos_x 	= AV_EARTHRADIUS * (AV_PI/180) * (ais.Ship[i].latitude-destination.latitude);
		ship_pos_y 	= AV_EARTHRADIUS * cos((destination.latitude * AV_PI/180)) * (AV_PI/180)
				    *(ais.Ship[i].longitude-destination.longitude);

		distance	= sqrt(pow(ship_pos_x - current_pos_x,2) + pow(ship_pos_y - current_pos_x,2));

#ifdef DEBUG_AISEVAL
                rtx_message("distance avalon to ship number %d is %f meters\n",i,distance);
                rtx_message("ship's position (x,y) = (%f,%f) \n",ship_pos_x, ship_pos_y);
#endif
		if ((distance < dist_of_sight) && (ais.Ship[i].speed_over_ground != 0.0)
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
                    heading_avalon     	= boatData.attitude.yaw*AV_PI/180.0;
                    angle_avalon_ship	= atan2((ship_pos_y-current_pos_y) , (ship_pos_x-current_pos_x));
		    angle_to_dest	= atan2((-current_pos_y),(-current_pos_x));

		    // take the average velocity over the last "num_speed_history" measurements
		    speed_avalon	= 0.5144444 * sqrt(pow(imu_clean.velocity.x,2) + pow(imu_clean.velocity.y,2));
                    speed_avalon_all.insert(speed_avalon_all.begin(),speed_avalon);
		    if(speed_avalon_all.size()>num_speed_history)
		      {speed_avalon_all.resize(num_speed_history);}
		    speed_avalon = 0;
		    for (l=0;l<speed_avalon_all.size();l++)
		      {speed_avalon = speed_avalon + 1.0/speed_avalon_all.size()*speed_avalon_all[l];}
		    speed_avalon_to_dest = speed_avalon*cos(angle_to_dest-heading_avalon); // TODO what velocity and angle do we take?
// rtx_message("velocity avalon abs: %f vel to dest: %f \n",speed_avalon,speed_to_dest);

                    vel_avalon_x	= cos(angle_to_dest) * speed_avalon_to_dest;
                    vel_avalon_y	= sin(angle_to_dest) * speed_avalon_to_dest;
                    vel_ship_x		= cos(heading_ship) * speed_ship;
                    vel_ship_y		= sin(heading_ship) * speed_ship;
                    vel_relativ_x	= vel_avalon_x - vel_ship_x;
                    vel_relativ_y	= vel_avalon_y - vel_ship_y;
		    angle_relativ	= atan2(vel_relativ_y, vel_relativ_x);
// rtx_message("avalon vel:  long= %f  lat= %f  total= %f  angle= %f", vel_avalon_long,vel_avalon_lat,speed_avalon_to_dest, angle_to_dest);
// rtx_message("relativ vel: long= %lf lat= %lf", vel_avalon_long-vel_ship_long,vel_avalon_lat-vel_ship_lat);


                    ship_length		= 1;  // ship length, if not from AIS
                    threshold_radius	= 495;  // to be modified: additional safety distance
                    radius_relativ	= ship_length + 4 + threshold_radius;    // ship length + avalon_length + threshold_radius
                    dist_avalon_ship	= sqrt(pow(ship_pos_x - current_pos_x,2) + pow(ship_pos_y - current_pos_y,2));

		    angle_tang_rear	= remainder((angle_avalon_ship - asin(radius_relativ/dist_avalon_ship)),2*AV_PI);
		    angle_tang_front	= remainder((angle_avalon_ship + asin(radius_relativ/dist_avalon_ship)),2*AV_PI);
		    


		    if (speed_avalon_to_dest < speed_ship)
		    {
rtx_message("avalon is slower\n"); 
			angle_crit_min = remainder((AV_PI+heading_ship - asin(speed_avalon_to_dest/speed_ship)),2*AV_PI);
			angle_crit_max = remainder((AV_PI+heading_ship + asin(speed_avalon_to_dest/speed_ship)),2*AV_PI);
rtx_message("ang_rear: %f  ang_front: %f  ang_min: %f  ang_max: %f",angle_tang_rear*180/AV_PI,angle_tang_front*180/AV_PI, angle_crit_min*180/AV_PI, angle_crit_max*180/AV_PI);
			if ((remainder((angle_crit_min-angle_tang_rear),2*AV_PI) < 0)
			  && (remainder((angle_crit_max-angle_tang_rear),2*AV_PI) > 0))
			{	// Calculate the two intersectin with the first tangent
			    obst_p.resize(num_obstP+2);
rtx_message("two obstacles at rear\n");  
			    obst_p[num_obstP].angle 	= remainder((angle_tang_rear
							  -asin(speed_avalon_to_dest/speed_ship*sin(AV_PI+heading_ship-angle_tang_rear))),2*AV_PI);
			    obst_p[num_obstP].t_crit	= ((current_pos_y-ship_pos_y)*(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle))
							  + (current_pos_x-ship_pos_x)*(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle)))
							  /(pow(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle),2)
							  +pow(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle),2));
			    obst_p[num_obstP].dist	= speed_avalon_to_dest * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].x		= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_x;
			    obst_p[num_obstP].y		= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_y;
			    num_obstP ++;


			    obst_p[num_obstP].angle 	= remainder((angle_tang_rear
							  +asin(speed_avalon_to_dest/speed_ship*sin(AV_PI+heading_ship-angle_tang_rear))-AV_PI),2*AV_PI);
			    obst_p[num_obstP].t_crit	= ((current_pos_y-ship_pos_y)*(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle))
							  + (current_pos_x-ship_pos_x)*(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle)))
							  /(pow(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle),2)
							  +pow(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle),2));
			    obst_p[num_obstP].dist	= speed_avalon_to_dest * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].x		= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_x;
			    obst_p[num_obstP].y		= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_y;
			    num_obstP ++;

			}

			if ((remainder((angle_crit_min-angle_tang_front),2*AV_PI) < 0) && (remainder((angle_crit_max-angle_tang_front),2*AV_PI) > 0))
			{	// Calculate the two intersectin with the second tangent
			    obst_p.resize(num_obstP+2);
rtx_message("two obstacles at front\n");  

			    obst_p[num_obstP].angle 	= remainder((angle_tang_front
							  -asin(speed_avalon_to_dest/speed_ship*sin(AV_PI+heading_ship-angle_tang_front))),2*AV_PI);
			    obst_p[num_obstP].t_crit	= ((current_pos_y-ship_pos_y)*(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle))
							  + (current_pos_x-ship_pos_x)*(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle)))
							  /(pow(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle),2)
							  +pow(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle),2));
			    obst_p[num_obstP].dist	= speed_avalon_to_dest * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].x		= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_x;
			    obst_p[num_obstP].y		= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_y;
			    num_obstP ++;
			    

			    obst_p[num_obstP].angle 	= remainder((angle_tang_front
							  +asin(speed_avalon_to_dest/speed_ship*sin(AV_PI+heading_ship-angle_tang_front))-AV_PI),2*AV_PI);
			    obst_p[num_obstP].t_crit	= ((current_pos_y-ship_pos_y)*(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle))
							  + (current_pos_x-ship_pos_x)*(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle)))
							  /(pow(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle),2)
							  +pow(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle),2));
			    obst_p[num_obstP].dist	= speed_avalon_to_dest * obst_p[num_obstP].t_crit + add_dist_safe;
			    obst_p[num_obstP].x		= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_x;
			    obst_p[num_obstP].y		= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_y;
			    num_obstP ++;
			}
		    }
		    else	// if speed_avalon_to_dest > speed_ship, we have an intersection with each tangent
		    {
rtx_message("avalon is faster\n"); 
			obst_p.resize(num_obstP+2);

			obst_p[num_obstP].angle 	= remainder((angle_tang_rear 
							  -asin(speed_avalon_to_dest/speed_ship*sin(AV_PI+heading_ship-angle_tang_rear))),2*AV_PI);
			obst_p[num_obstP].t_crit	= ((current_pos_y-ship_pos_y)*(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle))
							  + (current_pos_x-ship_pos_x)*(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle)))
							  /(pow(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle),2)
							  +pow(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle),2));
			obst_p[num_obstP].dist		= speed_avalon_to_dest * obst_p[num_obstP].t_crit + add_dist_safe;
			obst_p[num_obstP].x		= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_x;
			obst_p[num_obstP].y		= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_y;
			num_obstP ++;
			  
			
			obst_p[num_obstP].angle 	= remainder((angle_tang_front 
							  -asin(speed_avalon_to_dest/speed_ship*sin(AV_PI+heading_ship-angle_tang_front))-AV_PI),2*AV_PI);
			obst_p[num_obstP].t_crit	= ((current_pos_y-ship_pos_y)*(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle))
							  + (current_pos_x-ship_pos_x)*(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle)))
							  /(pow(vel_ship_y - speed_avalon_to_dest*sin(obst_p[num_obstP].angle),2)
							  +pow(vel_ship_x - speed_avalon_to_dest*cos(obst_p[num_obstP].angle),2));
			obst_p[num_obstP].dist		= speed_avalon_to_dest * obst_p[num_obstP].t_crit + add_dist_safe;
			obst_p[num_obstP].x		= obst_p[num_obstP].dist*cos(obst_p[num_obstP].angle) + current_pos_x;
			obst_p[num_obstP].y		= obst_p[num_obstP].dist*sin(obst_p[num_obstP].angle) + current_pos_y;
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
			check_angle = atan2(speed_avalon_to_dest*sin((obst_p[0].angle + obst_p[1].angle)/2)-vel_ship_y,
					    speed_avalon_to_dest*cos((obst_p[0].angle + obst_p[1].angle)/2)-vel_ship_x);
			if ((remainder((check_angle - angle_tang_rear),2*AV_PI)<=0) 
			  || (remainder((check_angle - angle_tang_front),2*AV_PI)>=0))
			{
			    temp_obst=obst_p[0];
			    for ( p = 0; p < num_obstP-1; p++)
			    {
			      obst_p[p]=obst_p[p+1];
			    }
			    obst_p[num_obstP-1]=temp_obst;
			}
		      
rtx_message("num_obst: %d    obst_p: %d\n",num_obstP,obst_p.size());
 
			// if an obstacle is too far away, we don't care about it
			//dist_dest = sqrt(pow(current_pos_x,2)+pow(current_pos_latitude,2));
			dist_limit = 2000;//2*dist_dest;
			for ( p = 0; p < num_obstP-1; p=p+2)
			{
 rtx_message("dest: %lf  limit: %lf  obst: front= %lf back= %lf",dist_dest, dist_limit, obst_p[p].dist, obst_p[p+1].dist);
			    if ((obst_p[p].dist<dist_limit) || (obst_p[p+1].dist<dist_limit))
			    {
				obst_p_start.push_back(obst_p[p]);
				obst_p_end.push_back(obst_p[p+1]);
			    }
			}
		    }
		}
	    }
	
rtx_message("numbers of obstacles: %d\n",obst_p_start.size());  
	    if (obst_p_start.size()==0){
		continue;
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
	    safe_destpoint_x	= obst_p_start[0].x; 
	    safe_destpoint_y	= obst_p_start[0].y;
	    dist_min		= obst_p_start[0].dist + sqrt(pow(obst_p_start[0].x,2) + pow(obst_p_start[0].y,2));
	    for (l=1;l<obst_p_start.size();l++)
	    {
		dist_test 	= obst_p_start[l].dist + sqrt(pow(obst_p_start[l].x,2) + pow(obst_p_start[l].y,2));
		if(dist_min>dist_test)
		{
		    dist_min=dist_test;
		    safe_destpoint_x	= obst_p_start[l].x; 
		    safe_destpoint_y	= obst_p_start[l].y;
		}
	    }
	    for (l=0;l<obst_p_end.size();l++)
	    {
		dist_test  	= obst_p_end[l].dist + sqrt(pow(obst_p_end[l].x,2) + pow(obst_p_end[l].y,2));
		if(dist_min>dist_test)
		{
		    dist_min=dist_test;
		    safe_destpoint_x	= obst_p_end[l].x; 
		    safe_destpoint_y	= obst_p_end[l].y;
		}
	    }

	    ais_dest.new_dest_long = destination.longitude+safe_destpoint_y*180/(AV_PI*AV_EARTHRADIUS*cos(destination.latitude*AV_PI/180.0));
	    ais_dest.new_dest_lat = destination.latitude+safe_destpoint_x*180.0/(AV_PI*AV_EARTHRADIUS);
	    ais_dest.ais_dest_index ++;
	    ais_dest.global_skipper_flag = AV_FLAGS_GLOBALSK_AVOIDANCE;
	    aisDestData.t_writefrom(ais_dest);
	    rtx_message("safe point local: x: %f, y: %f", safe_destpoint_x, safe_destpoint_y);
	    rtx_message("new destpoint: lat: %f, long: %f", ais_dest.new_dest_lat, ais_dest.new_dest_long);

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
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ShipStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ShipData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisDestData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
        DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));
        DOC(DDX_STORE_REGISTER_TYPE (store.getId(), SkipperFlags));
    //
	
    // Create output variable
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
        DOB(store.registerVariable(aisStruct, varname_aisStruct, "AisStruct"));
        DOB(store.registerVariable(aisData, varname_aisData, "AisData"));
	DOB(store.registerVariable(aisDestData, varname_aisDestData, "AisDestData"));
        DOB(store.registerVariable(dataBoat, varname4, "imuData"));
	DOB(store.registerVariable(dataImuClean, varname_imuClean, "imuCleanData"));
        DOB(store.registerVariable(shipStruct, varname_shipStruct, "ShipStruct"));
        DOB(store.registerVariable(shipData, varname_shipData, "ShipData"));
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
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
