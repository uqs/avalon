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
#include "coord_transform.h"
#include "destination.h"

#define DEBUG_NAVIGATOR

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataWindClean; //to get clean wind Data
DDXVariable naviData;  //to initialize the waypointData 
DDXVariable dataBoat; //do get imu-Data
DDXVariable waypointData; //to store the calculated path
DDXVariable dataFlags;
DDXVariable dataNaviFlags;
DDXVariable dataRcFlags;
DDXVariable transformationData; //public transformation data, could be not public??
DDXVariable destinationData;
DDXVariable destinationStruct; //the curr heading will be written here; sailor needs that!!!


/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(float i);
void setIsland(const char *islandFile, UISpace & map, LakeTransformation transformation, int calculation_iterator);
void prepareConnectivity16(Dijkstra3D::ConnectivityList & list);
void initializeHeadingTable16(std::vector<double> & headingTable);

/**
 * Storage for the command line arguments
 * */


const char * varname_navi = "navidata"; //does it have to be in the store as navidata, since its only the type of wypData?
const char * varname2 = "cleanwind";
const char * varname_wyp = "wypData";
const char * varname4 = "imu";
const char * varname_flags = "flags";
const char * varname_naviflags = "naviflags";
const char * varname_rcflags = "rcflags";
const char * varname_transf = "transfData";
const char * varname_destData = "destData";
const char * varname_destStruct = "destStruct";



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

    DestinationData destination;
    WindCleanData cleanedWind;
    imuData boatData;
    Waypoints waypoints;
    NaviData last_wyp_data;
    Flags generalflags;
	rcFlags rcflags;
    NaviFlags naviflags;
    LakeTransformation transformation;
    int xSize, ySize;
    double windDirection, windSpeed;
    double endTheta;
    int current_destpoint;
    int arrayPointer = 0;
    int i;
    int p,q;
    double difference, difference_start;
    int mapTheta_end_correct, mapTheta_start_correct;
    unsigned int start_calc;
    int calculation_iterator = 1;
    time_t start,end;
    double timedif=10.0;
    bool time_initializer = false;
	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (dataBoat.t_readto(boatData,10,1))
		{
			//naviData.t_readto(boatData,0,0); //not necessary, isn't it?
            waypointData.t_readto(waypoints,0,0);
            dataFlags.t_readto(generalflags,0,0);
            dataNaviFlags.t_readto(naviflags,0,0);
            dataRcFlags.t_readto(rcflags,0,0);
            transformationData.t_readto(transformation,0,0);
            destinationData.t_readto(destination,0,0);
            dataWindClean.t_readto(cleanedWind,0,0);

            if(time_initializer)
            {
            time(&end);
            timedif = difftime(end,start);
            }

            if((generalflags.navi_calculation == AV_FLAGS_NAVI_ENABLER_ON) && (generalflags.autonom_navigation) && (timedif > 7.0))  //if this is 1, then do the waypoint-calculation
            {
                time_initializer = true;

                //get the current_destpoint up to speed:
                for (i = 0; i<12; i++)
                {
                    if(destination.Data[i].passed == 1) continue;
                    current_destpoint = i;
                    break;
                }

#ifdef DEBUG_NAVIGATOR
                rtx_message("current_destinationpoint = %d \n",current_destpoint);
#endif

                start_calc = 1;
               
                /////////////////////MAIN WHILE TO GO THROUGH ////////////////////

                while(destination.Data[current_destpoint].type != AV_DEST_TYPE_NOMORE)
                {
                    //the whole story:

                    //converting the current and end-state into a meter-coordinates:
                    if(start_calc == 1)
                    {
                        transformation.longitude_start = boatData.position.longitude;
                        transformation.latitude_start = boatData.position.latitude;
                        transformation.longitude_end = destination.Data[current_destpoint].longitude;
                        transformation.latitude_end = destination.Data[current_destpoint].latitude;
                      
                        start_calc = 0;
                    }
                    else
                    {
                        transformation.longitude_start = destination.Data[current_destpoint-1].longitude;
                        transformation.latitude_start = destination.Data[current_destpoint-1].latitude;
                        transformation.longitude_end = destination.Data[current_destpoint].longitude;
                        transformation.latitude_end = destination.Data[current_destpoint].latitude;
                    }

#ifdef DEBUG_NAVIGATOR
                    rtx_message("gpscoordinates: start: %f, %f, end: %f, %f \n", boatData.position.longitude, boatData.position.latitude, transformation.longitude_end, transformation.latitude_end);
#endif 


                    //transformation into meter-coordinates:

                    transformation.longitude_start_transf = AV_EARTHRADIUS 
                        *cos((transformation.latitude_start * AV_PI/180))*(AV_PI/180)
                        *transformation.longitude_start;

                    transformation.latitude_start_transf = AV_EARTHRADIUS
                        *(AV_PI/180)*transformation.latitude_start;

                    transformation.longitude_end_transf = AV_EARTHRADIUS 
                        *cos((transformation.latitude_end * AV_PI/180))*(AV_PI/180)
                        *transformation.longitude_end;

                    transformation.latitude_end_transf = AV_EARTHRADIUS
                        *(AV_PI/180)*transformation.latitude_end;

                    //calculating the offsets:
                    transformation.x_offset = 5; //to be modified!
                    transformation.y_offset = 5;

#ifdef DEBUG_NAVIGATOR
                    rtx_message("transformation made!!");
#endif 

                    if ((transformation.longitude_end_transf - transformation.longitude_start_transf)>0)
                    {
                        transformation.longitude_offset = transformation.longitude_start_transf - transformation.x_offset*AV_NAVI_GRID_SIZE;
                        transformation.x_start = transformation.x_offset;
                        transformation.x_end = (transformation.longitude_end_transf - transformation.longitude_offset) / AV_NAVI_GRID_SIZE;
                    }
                    else
                    {
                        transformation.longitude_offset = transformation.longitude_end_transf - transformation.x_offset*AV_NAVI_GRID_SIZE;
                        transformation.x_end = transformation.x_offset;
                        transformation.x_start = (transformation.longitude_start_transf - transformation.longitude_offset) / AV_NAVI_GRID_SIZE;
                    }

                    //the same procedure for the y-coordinates:
                    if ((transformation.latitude_end_transf - transformation.latitude_start_transf)>0)
                    {
                        transformation.latitude_offset = transformation.latitude_start_transf - transformation.y_offset*AV_NAVI_GRID_SIZE;
                        transformation.y_start = transformation.y_offset;
                        transformation.y_end = (transformation.latitude_end_transf - transformation.latitude_offset) / AV_NAVI_GRID_SIZE;
                    }
                    else
                    {
                        transformation.latitude_offset = transformation.latitude_end_transf - transformation.y_offset*AV_NAVI_GRID_SIZE;
                        transformation.y_end = transformation.y_offset;
                        transformation.y_start = (transformation.latitude_start_transf - transformation.latitude_offset) / AV_NAVI_GRID_SIZE;
                    }

                    xSize = (2*transformation.x_offset + abs(transformation.x_end - transformation.x_start));
                    ySize = (2*transformation.y_offset + abs(transformation.y_end - transformation.y_start));    

                    if(transformation.x_start == transformation.x_end || transformation.y_start == transformation.y_end)
                    {
                        rtx_message("start and end have the same coordinates");
                        //TODO: something needs to happen here!
                        current_destpoint++;
                        continue;
                    }

#ifdef DEBUG_NAVIGATOR
                    rtx_message("2: xsize: %d, ysize %d; xstart = %d, xend = %d \n",xSize, ySize, transformation.x_start, transformation.x_end);
#endif

                    //initializing the grid:
                    UISpace vspace(0,100,xSize,
                            0,100,ySize,
                            -M_PI,M_PI,16);

                    //TODO 
                    ////////////////////////////////////////////////////////////////////////////////////////
                    //setIslands-Function

                    //template: setIsland(const char *islandFile, UISpace & map, LakeTransformation transformation)
                    setIsland("seekarte",vspace,transformation,calculation_iterator);

                    ///////////////////////////////////////////////////////////////////////////////////////

                    //transformation winddirection into correct mathematical direction:
                    windDirection = remainder(((-(cleanedWind.global_direction_real_long - 90))*(AV_PI / 180)),2*AV_PI);  //in rad and mathematically correct!!
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
                        endTheta = (- (AV_PI)/4);  ///works only for neighborhood 8!!!!!!!!!
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
                    
                    Dijkstra3D::Coordinate start(transformation.x_start, transformation.y_start, mapTheta_end_correct); //Achtung: changed to mapTheta_end!!!!
                    Dijkstra3D::Coordinate goal(transformation.x_end, transformation.y_end, mapTheta_start_correct);
                    AV_lce_Eval tunnelEval(vspace,goal,start);           // Heuristic eval, back to A*

                    Dijkstra3D::PathFinder<navi3dCell> pfinder(vspace, tunnelEval, transXEval, 
                            cylinderTrans, connectXEval);
                    
                    pfinder.setExhaustiveSearch(false); // Do we want to stop at the first hit to the goal

#ifdef DEBUG_NAVIGATOR
                    rtx_message("Starting search\n");
#endif

                    int res = pfinder.search(start, goal, path);

#ifdef DEBUG_NAVIGATOR
                    rtx_message("Search result %d Path size %d\n",res,path.size());
#endif

                    Dijkstra3D::ShortestPath::const_iterator it;

                    /////////////////////////////////////////////////////////////
                    FILE * pathfile;
                    char filename[20];
                    sprintf(filename,"pathsolution_%d",calculation_iterator);
                    pathfile = fopen(filename,"w");

                    for(it=path.begin();it!=path.end();it++)
                    {
                        fprintf(pathfile,"%d %d \n",it->x,it->y);
                    }
                    fclose(pathfile);
                    calculation_iterator++;

                    ////////////////////////////////////////////////////////////
                    //////////////writing the calculated waypoints into the store in an array
                    ////////////////////////////////////////////////////////////

                    for (it=path.begin();it!=path.end();it++) {
#ifdef DEBUG_NAVIGATOR
                        printf("x and y (%d,%d) at curr(%d) position\n",it->x,it->y,arrayPointer);
#endif
                        if(arrayPointer >= 100) break;

                        if(last_wyp_data.heading - (remainder((-(headingTable16[(it->theta)]*180/AV_PI)+90),360.0)) > 1e-2)
                        {
                            waypoints.Data[arrayPointer].longitude = last_wyp_data.longitude;
                            waypoints.Data[arrayPointer].latitude = last_wyp_data.latitude;
                            waypoints.Data[arrayPointer].heading = last_wyp_data.heading;
                            waypoints.Data[arrayPointer].wyp_type = AV_WYP_TYPE_PASSBY;
                            waypoints.Data[arrayPointer].windspeed = windSpeed;
                            waypoints.Data[arrayPointer].winddirection = windDirection;

                            arrayPointer++;
                        }

                        last_wyp_data.longitude = ((it->x)*AV_NAVI_GRID_SIZE+transformation.longitude_offset);
                        last_wyp_data.latitude = ((it->y)*AV_NAVI_GRID_SIZE+transformation.latitude_offset);
                        last_wyp_data.heading =remainder((-(headingTable16[(it->theta)]*180/AV_PI)+90),360.0);
                        last_wyp_data.wyp_type = AV_WYP_TYPE_PASSBY;
                        last_wyp_data.windspeed = windSpeed;
                        last_wyp_data.winddirection = windDirection;
                    }
                    
                    if(destination.Data[current_destpoint].type == AV_DEST_TYPE_END)
                    {
                        waypoints.Data[arrayPointer].longitude = last_wyp_data.longitude;
                        waypoints.Data[arrayPointer].latitude = last_wyp_data.latitude;
                        waypoints.Data[arrayPointer].heading = last_wyp_data.heading;
                        waypoints.Data[arrayPointer].wyp_type = AV_WYP_TYPE_END;
                        waypoints.Data[arrayPointer].windspeed = windSpeed;
                        waypoints.Data[arrayPointer].winddirection = windDirection;
                    }
                    else if(destination.Data[current_destpoint].type == AV_DEST_TYPE_BUOY)
                    {
                        waypoints.Data[arrayPointer].longitude = last_wyp_data.longitude;
                        waypoints.Data[arrayPointer].latitude = last_wyp_data.latitude;
                        waypoints.Data[arrayPointer].heading = last_wyp_data.heading;
                        waypoints.Data[arrayPointer].wyp_type = AV_WYP_TYPE_BUOY;
                        waypoints.Data[arrayPointer].windspeed = windSpeed;
                        waypoints.Data[arrayPointer].winddirection = windDirection;
                    }

                    //finished with that string
                    current_destpoint++;

                }
                waypointData.t_writefrom(waypoints);
                time(&start);
            }

            // Bring to store
            //dataNaviFlags.t_writefrom(naviflags);
            transformationData.t_writefrom(transformation);

		}

            //has to be modified


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
void setIsland(const char *islandFile, UISpace & map, LakeTransformation transformation,int calculation_iterator)
{
    //std::string line;
    std::ifstream island (islandFile);
    int xCurr, yCurr;
    int newX,newY;
    float longitude,latitude;
    int longitude_m,latitude_m;
    int xLongitude,yLatitude;
    FILE * islandplotter;
    char filename_islands[20];
    sprintf(filename_islands,"gnuplot_islands_%d",calculation_iterator);
    islandplotter = fopen(filename_islands,"w");

    char currentLine[10000], *ptr;
    if (island.is_open())
    {

        island.getline (currentLine,30,'\n');
        ////////////////////////////////
#ifdef DEBUG_ISLAND
        std::cout << "current Line: " << currentLine << std::endl;
#endif
        longitude = strtod(currentLine,&ptr);
        latitude = strtod(ptr,NULL);

        longitude_m = AV_EARTHRADIUS 
            *cos((latitude * AV_PI/180))*(AV_PI/180) *longitude;
        latitude_m = AV_EARTHRADIUS *(AV_PI/180)*latitude;
        /////////////////////////////

#ifdef DEBUG_ISLAND
        printf("longitude = %f, latitude = %f \n ",longitude, latitude);
#endif
        island.getline (currentLine,20,'\n');
#ifdef DEBUG_ISLAND
        std::cout << "current Line: " << currentLine << std::endl;
#endif
        island.getline (currentLine,20,'\n');
#ifdef DEBUG_ISLAND
        std::cout << "current Line: " << currentLine << std::endl;
#endif
        xLongitude = strtol(currentLine,&ptr,0);
        yLatitude = atoi(ptr);

#ifdef DEBUG_ISLAND
        printf("Xlongitude = %d, Xlatitude = %d \n",xLongitude, yLatitude);
#endif

        island.getline (currentLine,20,'\n');
#ifdef DEBUG_ISLAND
        std::cout << "current Line: " << currentLine << std::endl;
#endif
        //should be at the first coordinate:

        while(currentLine[0]!='#')
        {
            xCurr = strtol(currentLine,&ptr,0);
            yCurr = atoi(ptr);

#ifdef DEBUG_ISLAND
            printf("xCurr = %d, yCurr = %d \n",xCurr,yCurr);
#endif

            newX = (longitude_m - (xLongitude - xCurr)*AV_NAVI_GRID_SIZE) - transformation.longitude_offset;
            newY = (latitude_m - (yLatitude - yCurr)*AV_NAVI_GRID_SIZE) - transformation.latitude_offset;

            if(map.contains(newX,newY,0))
            {
                //block that cell:
                map(newX,newY,0).value = 20000;
                fprintf(islandplotter,"%d %d \n",newX,newY);
#ifdef DEBUG_ISLAND
                printf("if in setIslands went through\n");
#endif
            }
            island.getline(currentLine,20,'\n');
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

int main (int argc, char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("Navigator (Lake) Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), WindCleanData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), rcFlags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), NaviData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Waypoints));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), LakeTransformation));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));



	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataWindClean, varname2, "WindCleanData"));
    DOB(store.registerVariable(dataBoat, varname4, "imuData"));
	//flags:
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
	DOB(store.registerVariable(dataNaviFlags, varname_naviflags, "NaviFlags"));
	DOB(store.registerVariable(dataRcFlags, varname_rcflags, "rcFlags"));
	//navigation
	DOB(store.registerVariable(naviData, varname_navi, "NaviData"));
	DOB(store.registerVariable(waypointData, varname_wyp, "Waypoints"));
	//tranformation details:
	DOB(store.registerVariable(transformationData, varname_transf, "LakeTransformation"));
	//destination of AVALON:
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
	DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));


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
