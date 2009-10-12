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


#define DEBUG_AISEVAL

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataBoat; //to get imu-Data
DDXVariable shipData; //already prcessed information, showing the dangerous points of impact
DDXVariable shipStruct;
DDXVariable aisData; //actual ais data
DDXVariable aisStruct;

/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(double i);


/**
 * Storage for the command line arguments
 * */

const char * varname4 = "imu";
const char * varname_shipData = "shipData";
const char * varname_shipStruct = "shipStruct";
const char * varname_aisData = "aisData";
const char * varname_aisStruct = "aisStruct";

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
    ShipData ship; //transformed information about ship (ais)
    AisData ais;				//gps-struct

    double current_pos_longitude, current_pos_latitude; //already transformed and in meters
    int p, q, i;
    double ship_pos_longitude, ship_pos_latitude;
    double ship_pos_longitude_dynamic, ship_pos_latitude_dynamic;
    RtxTime time_now;
    double time_now_d;
    double distance_static, distance_dynamic;
    int simtime;
    double heading_ship;
    bool inserted;

    while (1) {
        // Read the next data available, or wait at most 5 seconds
        if (dataBoat.t_readto(boatData,10,1))
        {
            aisData.t_readto(ais,0,0);
            shipData.t_readto(ship,0,0);

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

            //go through all the ships in the vicinity and see if something
            //could be dangerous:
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
                    heading_ship = ais.Ship[i].heading;
                    if (heading_ship == 0.0)
                    {
                        heading_ship = ais.Ship[i].course_over_ground;
                    }
#ifdef DEBUG_AISEVAL
                    rtx_message("ship in reachable distance (20 km) \n");
#endif
                    ship_pos_longitude_dynamic = ship_pos_longitude;
                    ship_pos_latitude_dynamic = ship_pos_latitude;

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
#ifdef DEBUG_AISEVAL
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
                    }
                }
            }

            shipData.t_writefrom(ship);

        }


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


int main (int argc, char * argv[])
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
    //
	
    // Create output variable
	DOB(store.registerVariable(aisStruct, varname_aisStruct, "AisStruct"));
	DOB(store.registerVariable(aisData, varname_aisData, "AisData"));
    DOB(store.registerVariable(dataBoat, varname4, "imuData"));
    DOB(store.registerVariable(shipStruct, varname_shipStruct, "ShipStruct"));
    DOB(store.registerVariable(shipData, varname_shipData, "ShipData"));
	
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
