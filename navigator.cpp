/**
 * This calls the 3D navigator to calculate a path on a lake
 *
 **/

// General Project Constants
#include "avalon.h"
#include "Grid3D.h"
#include "GridTypes.h"
#include "movements3d.h"
#include <list>


// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <time.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

// Specific Things
#include "include/ddxjoystick.h"

#include <DDXStore.h>
#include <DDXVariable.h>

#include "windcleaner.h"
#include "windsensor.h"
#include "imu.h"
#include "flags.h"
#include "waypoints.h"
#include "transformation.h"
#include "destination.h"
#include "aisEval.h"

//#define DEBUG_ISLAND
// #define DEBUG_NAVIGATOR

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWindClean; //to get clean wind Data
DDXVariable waypointStruct;  //to initialize the waypointData 
DDXVariable dataBoat; //do get imu-Data
DDXVariable waypointData; //to store the calculated path
DDXVariable dataFlags;
DDXVariable dataNaviFlags;
DDXVariable dataRcFlags;
DDXVariable transformationData; //public transformation data, could be not public??
DDXVariable destinationData;
DDXVariable destinationStruct; //the curr heading will be written here; sailor needs that!!!
DDXVariable shipData_processed; //already prcessed information, showing the dangerous points of impact
DDXVariable shipStruct;


/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(float i);
void setIsland(const char *islandFile, UISpace & map, LakeTransformation transformation, int calculation_iterator);
void setAisObstacles(ShipData ship, UISpace & map, LakeTransformation transformation, int calculation_iterator);
void prepareConnectivity16(Dijkstra3D::ConnectivityList & list);
void initializeHeadingTable16(std::vector<double> & headingTable);


/**
 * Storage for the command line arguments
 * */


const char * varname_wypStruct = "wypStruct"; //does it have to be in the store as navidata, since its only the type of wypData?
const char * varname2 = "cleanwind";
const char * varname_wypData = "wypData";
const char * varname4 = "imu";
const char * varname_flags = "flags";
const char * varname_naviflags = "naviflags";
const char * varname_rcflags = "rcflags";
const char * varname_transf = "transfData";
const char * varname_destData = "destData";
const char * varname_destStruct = "destStruct";
const char * varname_shipData = "shipData";
const char * varname_shipStruct = "shipStruct";



const char * producerHelpStr = "activating the navi-algorithm (3D-Planning)";

/**
 * Command line arguments   //has yet to be completed
 *
 * */
RtxGetopt producerOpts[] = {
	{"cleanedWind", "Store Variable where the cleaned Wind data is read from",
		{
			{RTX_GETOPT_STR, &varname2, "WindCleanData"},
			RTX_GETOPT_END_ARG
		}
	},
	//  {"waypoint_array","Variable where the calculated navi-waypoints are stored",
	///   {
	//  {RTX_GETOPT_STR, &varname_dest, "Waypoints"},
	//   RTX_GETOPT_END_ARG
	//  }
	// },
	{"imuData", "Store Variable where the imuData is written",
		{
			{RTX_GETOPT_STR, &varname4, "imuData"},
			RTX_GETOPT_END_ARG
		}
	},
	RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{
	Dijkstra3D::ShortestPath path;
	std::vector<double> headingTable16;
	Dijkstra3D::ConnectivityList connectivity16;

	ShipData ship; //transformed information about ship (ais)
	DestinationData destination;
	WindCleanData cleanedWind;
	imuData boatData;
	WaypointData waypoints;
	WaypointStruct wyp_data,last_wyp_data;
	Flags generalflags;
	rcFlags rcflags;
	NaviFlags naviflags;
	LakeTransformation transformation;

	int xSize, ySize;
	double windDirection, windSpeed;
	double endTheta;
	int arrayPointer = 0;
	int p,q;
	double difference, difference_start;
	int mapTheta_end_correct, mapTheta_start_correct;
	int calculation_iterator = 1;
	time_t start_time,end;
	double timedif=10.0;
	bool time_initializer = false;
	unsigned int last_calc_index = 1234567; //TODO change to some decent number
	unsigned int last_skip_index = 0;
	unsigned int max_num_wyp = 0;

	//initializing the answer index: 
	dataNaviFlags.t_readto(naviflags,0,0);
	naviflags.navi_index_answer = 1234567;
	dataNaviFlags.t_writefrom(naviflags);


	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (dataBoat.t_readto(boatData,10,1))
		{
			//             naviData.t_readto(boatData,0,0); //not necessary, isn't it?
			waypointData.t_readto(waypoints,0,0);
			dataFlags.t_readto(generalflags,0,0);
			dataNaviFlags.t_readto(naviflags,0,0);
			dataRcFlags.t_readto(rcflags,0,0);
			transformationData.t_readto(transformation,0,0);
			destinationData.t_readto(destination,0,0);
			dataWindClean.t_readto(cleanedWind,0,0);
			shipData_processed.t_readto(ship,0,0);

			if(time_initializer)
			{
				time(&end);
				timedif = difftime(end,start_time);
			}
			// rtx_message("last_calc_ind: %d    navi_call_ind: %d\n",last_calc_index,generalflags.navi_index_call);
			if(((last_calc_index != generalflags.navi_index_call) || (last_skip_index != generalflags.skip_index_dest_call))
					&& (generalflags.autonom_navigation)) 
				//&& (timedif > 7.0))  //if this is 1, then do the waypoint-calculation
			{
				time_initializer = true;
				rtx_message("new_path_calc!!!\n");

				//converting the current and end-state into a meter-coordinates:
				transformation.longitude_start = boatData.position.longitude;
				transformation.latitude_start = boatData.position.latitude;


				//transformation into meter-coordinates:

				transformation.longitude_start_transf = AV_EARTHRADIUS 
					*cos((transformation.latitude_start * AV_PI/180))*(AV_PI/180)
					*transformation.longitude_start;

				transformation.latitude_start_transf = AV_EARTHRADIUS
					*(AV_PI/180)*transformation.latitude_start;


				//calculating the offsets:
				transformation.x_offset = 5; //to be modified!
				transformation.y_offset = 5;
#ifdef DEBUG_NAVIGATOR
				rtx_message("start: %f %f m; end: %f %f m \n",transformation.longitude_start_transf, transformation.latitude_start_transf,
						destination.longitude, destination.latitude);
#endif 
				// rtx_message("start: %f %f m; end: %f %f m \n",transformation.longitude_start_transf+4.380225573914934e+06, transformation.latitude_start_transf-1.111949403453934e+06,
				//                         destination.longitude+4.380225573914934e+06, destination.latitude-1.111949403453934e+06);

				if ((destination.longitude - transformation.longitude_start_transf)>0)
				{
					transformation.longitude_offset = (int)(transformation.longitude_start_transf) - transformation.x_offset*AV_NAVI_GRID_SIZE;
					transformation.x_start = transformation.x_offset;
					transformation.x_end = (int)((destination.longitude - (double)(transformation.longitude_offset)) / (double)AV_NAVI_GRID_SIZE);
				}
				else
				{
					transformation.longitude_offset = destination.longitude - transformation.x_offset*AV_NAVI_GRID_SIZE;
					transformation.x_end = transformation.x_offset;
					transformation.x_start = (transformation.longitude_start_transf - transformation.longitude_offset) / AV_NAVI_GRID_SIZE;
				}

				//the same procedure for the y-coordinates:
				if ((destination.latitude - transformation.latitude_start_transf)>0)
				{
					transformation.latitude_offset = transformation.latitude_start_transf - transformation.y_offset*AV_NAVI_GRID_SIZE;
					transformation.y_start = transformation.y_offset;
					transformation.y_end = (destination.latitude - transformation.latitude_offset) / AV_NAVI_GRID_SIZE;
				}
				else
				{
					transformation.latitude_offset = destination.latitude - transformation.y_offset*AV_NAVI_GRID_SIZE;
					transformation.y_end = transformation.y_offset;
					transformation.y_start = (transformation.latitude_start_transf - transformation.latitude_offset) / AV_NAVI_GRID_SIZE;
				}

				xSize = (2*transformation.x_offset + abs(transformation.x_end - transformation.x_start));
				ySize = (2*transformation.y_offset + abs(transformation.y_end - transformation.y_start));    

				if(transformation.x_start == transformation.x_end && transformation.y_start == transformation.y_end)
				{
					rtx_message("start and end have the same coordinates");
					continue;
				}

				#ifdef DEBUG_NAVIGATOR
				rtx_message("2: xsize: %d, ysize %d; xstart = %d, xend = %d, ystart = %d, yend = %d \n",xSize, ySize, transformation.x_start, transformation.x_end, transformation.y_start, transformation.y_end);
				#endif

				//initializing the grid:
				UISpace vspace(0,100,xSize,
						0,100,ySize,
						-M_PI,M_PI,16);

				//TODO 
				////////////////////////////////////////////////////////////////////////////////////////
				//setIslands-Function

				//template: setIsland(const char *islandFile, UISpace & map, LakeTransformation transformation)
				//setIsland("seekarte",vspace,transformation,calculation_iterator);
				//setAisObstacles(ship, vspace, transformation, calculation_iterator); //to get out of the way of ships

				//rtx_message("HERE: this should read 40'000 : -> %d <- \n",vspace(50,50,0).value);
				///////////////////////////////////////////////////////////////////////////////////////

				//transformation winddirection into correct mathematical direction:
				windDirection = remainder(((-(cleanedWind.global_direction_real_long - 90))*(AV_PI / 180.0)),2*AV_PI);  //in rad and mathematically correct!!
				windSpeed = cleanedWind.speed_long;    	//in knots

				#ifdef DEBUG_NAVIGATOR
				rtx_message("windspeed = %f, winddirection = %f\n",windSpeed,windDirection);
				#endif

				///////////////////////////////////////////////////////////////////////////////////////////
				//initialize some things:

				initializeHeadingTable16(headingTable16);
				prepareConnectivity16(connectivity16);


				///////////////////////////////////////////////////////////////////////////////////////////
				//calculating an appropriate endTheta:
				//
				endTheta = atan2((transformation.y_end - transformation.y_start),(transformation.x_end - transformation.x_start));

				if (fabs(remainder((windDirection - endTheta),2*AV_PI))== 0.0)
				{
					endTheta = (- (AV_PI)/4); 
				}
				if ((fabs(remainder((windDirection - endTheta),2*AV_PI)) < (AV_PI/4 - 0.5*AV_PI/180)) &&  (remainder((windDirection - endTheta),2*AV_PI) > 0))
				{
					endTheta = (windDirection -  67.5*AV_PI/180);
				}
				if ((fabs(remainder((windDirection - endTheta),2*AV_PI)) < (AV_PI/4 - 0.5*AV_PI/180)) &&  (remainder((windDirection - endTheta),2*AV_PI) < 0))
				{
					endTheta =(windDirection +  67.5*AV_PI/180); //(AV_PI)*(1/4+1/NEIGHBORHOOD));
				}

				//if endTheta is out of range:
				endTheta = remainder(endTheta,2*AV_PI);

				///modifying the endTheta so its in our headingTable:
				difference = AV_PI/2;
				mapTheta_end_correct = 30; //more than it can ever be possible
				for(p=0; p<16; p++)
				{
					if (fabs(remainder((headingTable16[p] - endTheta),2*AV_PI)) < difference)
					{
						difference = fabs(remainder((headingTable16[p] - endTheta),2*AV_PI));
						mapTheta_end_correct = p;
					}
				}
				//-----> theta at the end is mapTheta_end_correct!!
#if 1
				//modifying the startTheta so its in our headingTable:
				difference_start = AV_PI/2;
				mapTheta_start_correct = 30; //more than it can ever be possible
				for(q=0; q<16; q++)
				{
					if (fabs(remainder((headingTable16[q] - boatData.attitude.yaw*AV_PI/180),2*AV_PI)) < difference_start)
					{
						difference_start = fabs(remainder((headingTable16[q] - boatData.attitude.yaw*AV_PI/180),2*AV_PI));
						mapTheta_start_correct = q;
					}
				}
				//-----> theta at the start is mapTheta_start_correct!!
// rtx_message("start_theta = %d \n",mapTheta_start_correct);
#ifdef DEBUG_NAVIGATOR
				rtx_message("start_theta = %d \n",mapTheta_start_correct);
#endif
#endif
				//////////////////////////////////////////////////////////////////////////////////////////


				AV_TransEval transXEval(headingTable16, windSpeed, windDirection);
				AV_TransApply cylinderTrans(vspace,headingTable16);
				AV_ConnectEval connectXEval(headingTable16);

				//default stuff:
				//Dijkstra3D::TransitionApplicator defaultTrans;
				//Dijkstra3D::CellEvaluator defaultEval; // Default eval, all cell cost are 0

				Dijkstra3D::Coordinate start(transformation.x_start, transformation.y_start, mapTheta_start_correct); //Achtung: changed to mapTheta_end!!!!
				Dijkstra3D::Coordinate goal(transformation.x_end, transformation.y_end, mapTheta_end_correct);
				AV_lce_Eval tunnelEval(vspace,goal,start);           // Heuristic eval, back to A*

				Dijkstra3D::PathFinder<navi3dCell> pfinder(vspace, tunnelEval, transXEval, 
						cylinderTrans, connectXEval);

				pfinder.setExhaustiveSearch(false); // Do we want to stop at the first hit to the goal

#ifdef DEBUG_NAVIGATOR
				rtx_message("Starting search\n");
#endif

				int res;
				res = pfinder.search(start, goal, path);

#ifdef DEBUG_NAVIGATOR
				rtx_message("Search result %d Path size %d\n",res,path.size());
#endif

				Dijkstra3D::ShortestPath::const_iterator it;

				/////////////////////////////////////////////////////////////
				FILE * pathfile;
				char filename[20];
#if 0
				sprintf(filename,"path_%d",calculation_iterator);
				pathfile = fopen(filename,"w");

				for (it=path.begin();it!=path.end();it++) {
					double longitude = ((it->x)*AV_NAVI_GRID_SIZE+transformation.longitude_offset);
					double latitude = ((it->y)*AV_NAVI_GRID_SIZE+transformation.latitude_offset);
					double heading =remainder((-(headingTable16[(it->theta)]*180/AV_PI)+90),360.0);
					fprintf(pathfile,"%d %d %e %e %e\n",it->x, it->y, longitude, latitude, heading);
				}
				fclose(pathfile);
#endif
				sprintf(filename,"pathsolution_%d",calculation_iterator);
				pathfile = fopen(filename,"w");
				////////////////////////////////////////////////////////////
				//////////////writing the calculated waypoints into the store in an array
				////////////////////////////////////////////////////////////

				last_wyp_data.heading = headingTable16[mapTheta_start_correct]; 
				arrayPointer = 0;
// int coun=-1;
printf("theta start: %f     current heading: %f\n",last_wyp_data.heading*180/AV_PI,boatData.attitude.yaw);
				for (it=path.begin();it!=path.end();it++) {
#ifdef DEBUG_NAVIGATOR
					printf("x and y (%d,%d) at curr(%d) position, heading = %f \n",it->x,it->y,arrayPointer,
							(remainder((-(headingTable16[(it->theta)]*180/AV_PI)+90.0),360.0)));
					printf("stupid difference: %f \n",
							(last_wyp_data.heading - (remainder((-(headingTable16[(it->theta)]*180/AV_PI)+90.0),360.0))));
#endif
					if(arrayPointer >= 100) break;
// coun++;
					wyp_data.longitude = ((it->x)*AV_NAVI_GRID_SIZE+transformation.longitude_offset);
					wyp_data.latitude = ((it->y)*AV_NAVI_GRID_SIZE+transformation.latitude_offset);
					wyp_data.heading =remainder((-(headingTable16[(it->theta)]*180/AV_PI)+90),360.0);
					wyp_data.wyp_type = AV_WYP_TYPE_PASSBY;
					wyp_data.windspeed = windSpeed;
					wyp_data.winddirection = remainder(-(windDirection - AV_PI/2),2*AV_PI)*180/AV_PI;



					if(arrayPointer == 0)
					{
						waypoints.Data[arrayPointer].longitude = wyp_data.longitude;
						waypoints.Data[arrayPointer].latitude = wyp_data.latitude;

						fprintf(pathfile,"%d %d %f\n",wyp_data.longitude,wyp_data.latitude, wyp_data.heading);/**//**/
// rtx_message("count: %d",coun);
						waypoints.Data[arrayPointer].heading = wyp_data.heading;
						waypoints.Data[arrayPointer].wyp_type = AV_WYP_TYPE_PASSBY;
						waypoints.Data[arrayPointer].passed = 1;
						waypoints.Data[arrayPointer].windspeed = windSpeed;
						waypoints.Data[arrayPointer].winddirection = remainder(-(windDirection - AV_PI/2),2*AV_PI)*180/AV_PI;

						arrayPointer = 1;
					} else if((fabs(remainder(wyp_data.heading - last_wyp_data.heading, 360.)) > 1e-2) 
							&& ((waypoints.Data[arrayPointer-1].longitude != wyp_data.longitude)
								|| (waypoints.Data[arrayPointer - 1].latitude != wyp_data.latitude))
							&& ((waypoints.Data[0].longitude != last_wyp_data.longitude)	//because we always had the first wyp twice
								|| (waypoints.Data[0].latitude != last_wyp_data.latitude)))
					{
#ifdef DEBUG_NAVIGATOR
						printf("inserted into store: last wyp \n");
#endif
						waypoints.Data[arrayPointer].longitude = last_wyp_data.longitude;
						waypoints.Data[arrayPointer].latitude = last_wyp_data.latitude;
// rtx_message("count: %d",coun);
// rtx_message("diff head: %f, head_curr: %f  head_last: %f",fabs(remainder(wyp_data.heading - last_wyp_data.heading, 360.)), wyp_data.heading,last_wyp_data.heading );
						fprintf(pathfile,"%d %d %f\n",last_wyp_data.longitude,last_wyp_data.latitude, last_wyp_data.heading);

						waypoints.Data[arrayPointer].heading = last_wyp_data.heading;
						waypoints.Data[arrayPointer].wyp_type = AV_WYP_TYPE_PASSBY;
						waypoints.Data[arrayPointer].passed = 0;
						waypoints.Data[arrayPointer].windspeed = windSpeed;
						waypoints.Data[arrayPointer].winddirection = remainder(-(windDirection - AV_PI/2),2*AV_PI)*180/AV_PI;

						arrayPointer++;
					}

					last_wyp_data = wyp_data;
				}

// 				if (arrayPointer < max_num_wyp)
// 				{
// 				    for (q=arrayPointer;q<max_num_wyp;q++) {
// 					
				waypoints.Data[arrayPointer].longitude = last_wyp_data.longitude;
				waypoints.Data[arrayPointer].latitude = last_wyp_data.latitude;
				waypoints.Data[arrayPointer].heading = last_wyp_data.heading;


				fprintf(pathfile,"%d %d %f\n",last_wyp_data.longitude,last_wyp_data.latitude, last_wyp_data.heading);
				waypoints.Data[arrayPointer].wyp_type = AV_WYP_TYPE_END;
				waypoints.Data[arrayPointer].passed = 0;
				waypoints.Data[arrayPointer].windspeed = windSpeed;
				waypoints.Data[arrayPointer].winddirection = remainder(-(windDirection - AV_PI/2),2*AV_PI)*180/AV_PI;


				fclose(pathfile);
				calculation_iterator++;
				//finished with that string

				last_calc_index = generalflags.navi_index_call;
				last_skip_index = generalflags.skip_index_dest_call;
				naviflags.navi_index_answer = generalflags.navi_index_call;
				dataNaviFlags.t_writefrom(naviflags);
				transformationData.t_writefrom(transformation);
				waypointData.t_writefrom(waypoints);
				time(&start_time);
rtx_message("path calc done");
			}

		} else if (dataWindClean.hasTimedOut()) {
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
		rtx_timer_sleep(3);
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

int sign(float i) // gives back the sign of a float
{
	if (i>=0)
		return 1;
	else
		return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//VIP Functions for the path planner
/////////////////////////////////////////////////////////////////////////////////////////////////
void setAisObstacles(ShipData ship, UISpace & map, LakeTransformation transformation,int calculation_iterator)
{
	int shipiterator, shiplat, shiplong;
	int dangerpoint_x, dangerpoint_y;
	FILE * islandplotter;
	char filename_islands[20];
	sprintf(filename_islands,"ais_islands_%d",calculation_iterator);
	islandplotter = fopen(filename_islands,"w");

	for(shipiterator = 0; shipiterator < ship.shipCount; shipiterator++)
	{
		for(shiplong = -500; shiplong < 510; shiplong += AV_NAVI_GRID_SIZE)
		{
			for(shiplat = -500; shiplat < 530; shiplat += AV_NAVI_GRID_SIZE)
			{
				dangerpoint_x = int (rint((ship.Data[shipiterator].longitude + shiplong - transformation.longitude_offset) 
							/ AV_NAVI_GRID_SIZE));
				dangerpoint_y = int (rint((ship.Data[shipiterator].latitude + shiplat - transformation.latitude_offset) 
							/ AV_NAVI_GRID_SIZE));

				if(map.contains(dangerpoint_x,dangerpoint_y,0))
				{
					//block that cell:
					map(dangerpoint_x,dangerpoint_y,0).value = 40000;
					fprintf(islandplotter,"%d %d \n",dangerpoint_x,dangerpoint_y);

				}
			}
		}
	}

	fclose(islandplotter);
}
void setIsland(const char *islandFile, UISpace & map, LakeTransformation transformation,int calculation_iterator)
{
	//std::string line;
	std::ifstream island (islandFile);
	int newX,newY;
	int m;
	double longitude_curr_old, latitude_curr_old;
	double longitude_curr, latitude_curr;
	FILE * islandplotter;
	char filename_islands[20];
	sprintf(filename_islands,"gnuplot_islands_%d",calculation_iterator);
	islandplotter = fopen(filename_islands,"w");

	char currentLine[10000], *ptr;
	if (island.is_open())
	{

		island.getline (currentLine,30,'\n');

		longitude_curr = strtod(currentLine,&ptr);
		latitude_curr = strtod(ptr,NULL);
		island.getline (currentLine,30,'\n');

		while(currentLine[0]!='#')
		{
			longitude_curr_old = longitude_curr;
			latitude_curr_old = latitude_curr_old;

			longitude_curr = strtod(currentLine,&ptr);
			latitude_curr = strtod(ptr,NULL);
#ifdef DEBUG_ISLAND
			printf("longitude = %f; latitude = %f \n",longitude_curr,latitude_curr);
#endif

			newX = int (rint((longitude_curr_old - transformation.longitude_offset) / AV_NAVI_GRID_SIZE));
			newY = int (rint((latitude_curr_old - transformation.latitude_offset) / AV_NAVI_GRID_SIZE));

			for( m = 0; m < 1000; m++)
			{
				newX += int (rint(double ((longitude_curr - longitude_curr_old) / AV_NAVI_GRID_SIZE *m /999.0)));
				newY += int (rint(double ((latitude_curr - latitude_curr_old) / AV_NAVI_GRID_SIZE *m / 999.0)));

#ifdef DEBUG_ISLAND
				printf("newX = %d; newY = %d ; m=%d \n",newX,newY,m);
#endif

				if(map.contains(newX,newY,0))
				{
					//block that cell:
					map(newX,newY,0).value = 100000;
					fprintf(islandplotter,"%d %d \n",newX,newY);
#ifdef DEBUG_ISLAND
					printf("if in setIslands went through\n");
#endif
				}
			}
			island.getline(currentLine,30,'\n');
		}
	}
	else std::cout << "Unable to open file";
	fclose(islandplotter);
}
////////////////////////////////////////////////////////////////
void prepareConnectivity16(Dijkstra3D::ConnectivityList & list) {
	list.clear();
	int k=0;
	unsigned int p;

	std::vector<double> headings;

#ifdef DEBUG_NEIGHBORHOOD
	printf("neighborhood16 did go through");
#endif //debugging

	for (int i=-2; i<3; i++)
	{
		for (int j=-2; j<3; j++)
		{
			//that results into 24 neighbors with 16 different headings:
			if ((i==0) && (j==0))
			{
				continue;
			}
			else
			{
				Dijkstra3D::Coordinate neighborNow;
				neighborNow.x =i;
				neighborNow.y =j;
				if(!k)
				{
					neighborNow.theta=k;
					headings.push_back(atan2(j,i));
					k++;
					list.push_back(neighborNow);
					continue;
				}

				for (p = 0; p<headings.size(); p++)
				{
					if(fabs(remainder(headings[p] - atan2(j,i),2*AV_PI)) < 1e-5 )
					{
						neighborNow.theta = p;
						break;
					}

				}
				if (p == headings.size())
				{
					neighborNow.theta = k;
					headings.push_back(atan2(j,i));
				}
				k++;
#ifdef DEBUG_NEIGHBORHOOD
				printf("neighborhood_size = %d , should go up to 16\n",headings.size());
#endif

				list.push_back (neighborNow);
			}
		}
	}
}
/////////////////////////////////////////////////////////

void initializeHeadingTable16(std::vector<double> & headingTable)
{
	headingTable.clear();
	for (int i=-2; i<3; i++)
	{
		for (int j=-2; j<3; j++)
		{
#ifdef DEBUG_HEADING_INIT
			printf("headingTable init - i: %d, j=%d \n",i,j);
#endif
			if ((i==-1 || i==0 || i==1) && (j==-1 || j==0 || j==1)) continue;
#ifdef DEBUG_HEADING_INIT
			printf("headingTable init after if - i: %d, j=%d \n",i,j);
			printf("gepushed wird: %f \n",atan2(j,i));
#endif
			headingTable.push_back(atan2(j,i));
		}
	}
#ifdef DEBUG_HEADING_INIT
	printf("size of the headingTable: %d \n",headingTable.size());
#endif
}
/////////////////////////////////////////////////////////////

int main (int argc, const char * argv[])
{
	RtxThread * th;
	int ret;

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Navigator Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rcFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WaypointStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WaypointData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), LakeTransformation));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ShipStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), ShipData));



	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataWindClean, varname2, "WindCleanData"));
	DOB(store.registerVariable(dataBoat, varname4, "imuData"));
	//flags:
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
	DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));
	DOB(store.registerVariable(dataRcFlags, varname_rcflags, "rcFlags"));
	//navigation
	DOB(store.registerVariable(waypointStruct, varname_wypStruct, "WaypointStruct"));
	DOB(store.registerVariable(waypointData, varname_wypData, "WaypointData"));
	//tranformation details:
	DOB(store.registerVariable(transformationData, varname_transf, "LakeTransformation"));
	//destination of AVALON:
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
	DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));
	//info about dangerous points of impact
	DOB(store.registerVariable(shipStruct, varname_shipStruct, "ShipStruct"));
	DOB(store.registerVariable(shipData_processed, varname_shipData, "ShipData"));


	// Start the working thread
	DOP(th = rtx_thread_create ("Navigator thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
				RTX_THREAD_CANCEL_DEFERRED,
				translation_thread, NULL,
				NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down navigator...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
	return (0);
}
