/**
 * calls the 3D navigation algorithm
 * */

#include "avalon.h"
#include "Grid3D.h"
#include "GridTypes.h"
#include "movements3d.h"
#include <list>

//#define DEBUG_HEADING_INIT
//#define DEBUG_ISLAND

//for setIslands
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>

#ifdef GNUPLOT_ENABLED  
#include <CGnuplot.h>
#endif

void initializeHeadingTable8(std::vector<double> & headingTable)
{
    headingTable.clear();
    
    for(int i=0; i<8; i++)
    {
        headingTable.push_back(remainder(i*AV_PI/4,2*AV_PI));
    }
}
    
#if 0
    headingTable.clear();
    for (int i=-1; i<2; i++)
    {
        for (int j=-1; j<2; j++)
        {
            if ((i==0) && (j==0))
            {
                continue;
            }
            else
            {
                headingTable.push_back(atan2(j,i));

            }
        }
    }
}
#endif

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

void initializeNeighborhood8(std::vector<Neighbor> & neighborhood) {
    neighborhood.clear();
#ifdef DEBUG_NEIGHBORHOOD 
    printf("neighborhood8 did go through");
#endif //debugging

    for (int i=-1; i<2; i++)
    {
        for (int j=-1; j<2; j++)
        {
            if ((i==0) && (j==0))
            {
                continue;
            }
            else
            {
                Neighbor neighborNow;
                neighborNow.x =i;
                neighborNow.y =j;

                neighborNow.traversedCells.push_back(std::pair<int,int>(i,j));
                neighborNow.traversedCells.push_back(std::pair<int,int>(0,0));
                neighborhood.push_back (neighborNow);
            }
        }
    }
}

void initializeNeighborhood16(std::vector<Neighbor> & neighborhood) {
    neighborhood.clear();

    int z;
    double deltax, deltay;
    int currentX, currentY;

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
                Neighbor neighborNow;
                neighborNow.x =i;
                neighborNow.y =j;

                // first time filling:
                if(i==-2 && j==-2)
                {
                    neighborNow.traversedCells.push_back(std::pair<int,int>(0,0));
                }

                deltax = 0.0;
                deltay = 0.0;
                z = 0;

                while (z<5)
                {
                    deltax += ((double)i)/5;
                    deltay += ((double)j)/5;

                    currentX = rint(deltax);
                    currentY = rint(deltay);

                    for (unsigned int q=0; q < neighborNow.traversedCells.size(); q++)
                    {
                        if(neighborNow.traversedCells[q].first == currentX
                                && neighborNow.traversedCells[q].second == currentY){
                            break;
                        }
                        neighborNow.traversedCells.push_back(std::pair<int,int>(currentX,currentY));
                    }
                    z++;
                }

                neighborhood.push_back (neighborNow);
            }
        }
    }
}

void prepareConnectivity8(Dijkstra3D::ConnectivityList & list) {
    list.clear();
    int k = 0;

#ifdef DEBUG_NEIGHBORHOOD
    printf("neighborhood8 did go through");
#endif //debugging

    for (int i=-1; i<2; i++)
    {
        for (int j=-1; j<2; j++)
        {
            if ((i==0) && (j==0))
            {
                continue;
            }
            else
            {
                Dijkstra3D::Coordinate neighborNow;
                neighborNow.x=i;
                neighborNow.y =j;
                neighborNow.theta=k;

                list.push_back (neighborNow);
                k++;

            }
        }
    }
}

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

void setIsland(const char *islandFile, UISpace & map)
{
    //std::string line;
    std::ifstream island (islandFile);
    int xCurr, yCurr;
    int newX,newY;
    float longitude,latitude;
    int longitude_m,latitude_m;
    int xLongitude,yLatitude;
    int lon_offset = 0;
    int lat_offset = 0;
    FILE * islandplotter;
    islandplotter = fopen("gnuplot_islands","w");

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

        longitude_m =int (AV_EARTHRADIUS 
            *cos((latitude * AV_PI/180))*(AV_PI/180) *longitude);
        latitude_m =int (AV_EARTHRADIUS *(AV_PI/180)*latitude);
        /////////////////////////////

#ifdef DEBUG_ISLAND
            printf("lon_offset = %d, lat_offset = %d \n",lon_offset, lat_offset);
#endif
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

            newX = (longitude_m - (xLongitude - xCurr)*AV_NAVI_GRID_SIZE) - lon_offset;
            newY = (latitude_m - (yLatitude - yCurr)*AV_NAVI_GRID_SIZE) - lat_offset;

            if(map.contains(newX,newY,0))
            {
                //block that cell:
                map(newX,newY,0).value = 200000;
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

int main(int argc,char * argv[]) 
{
    int x_start,y_start;
    int x_end,y_end;
    double difference, endTheta;
    int mapTheta_end_correct, p;
    
    #ifdef GNUPLOT_ENABLED
    CGnuplot gpl;
    #endif //GNUPLOT_ENABLED
    
    
    UISpace vspace(0,100,60,
            0,100,60,
			-M_PI,M_PI,16);

    //set the islands:
    setIsland("ba_setup4.txt",vspace);

#ifdef DEBUG_MAIN
    printf("printing the damn island: %d at %d,%d,%d \n",vspace(1,2,1).value,1,2,1);
#endif

    double windDirection = remainder((-(atof(argv[5])*AV_PI/180 - AV_PI/2)),2*AV_PI); //in mathematical degrees
    double windSpeed = 15.0; //knots


	//vspace.print();
   

    //std::vector<double> headingTable8;
    std::vector<double> headingTable16;
    //std::vector<Neighbor> neighborhood8;
    //std::vector<Neighbor> neighborhood16;

    //initializeHeadingTable8(headingTable8);
    initializeHeadingTable16(headingTable16);
    //initializeNeighborhood8(neighborhood8);
    //initializeNeighborhood16(neighborhood16);

    
    Dijkstra3D::ShortestPath path;
	Dijkstra3D::ConnectivityList connectivity6;
	Dijkstra3D::ConnectivityList connectivity8;
	Dijkstra3D::ConnectivityList connectivity16;
	
    //initializeConnectivity8(connectivity8);
	//initializeConnectivity16(connectivity16);
	
    //Dijkstra3D::prepareConnectivity6(connectivity6);
    prepareConnectivity8(connectivity8);
	prepareConnectivity16(connectivity16);
	//AV_TransEval trans6Eval(connectivity6, headingTable8, windSpeed, windDirection); // Evaluate the cost of a 4 connectivity
	//AV_TransEval trans8Eval(connectivity8, headingTable8, windSpeed, windDirection); // Evaluate the cost of a 4 connectivity
	AV_TransEval transXEval(headingTable16, windSpeed, windDirection);
	//AV_TransEval trans16Eval(connectivity16, headingTable16, windSpeed, windDirection); // Evaluate the cost of a 8 connectivity

    //TODO: find better 
    x_start = atoi(argv[1]);
    y_start = atoi(argv[2]);
    x_end = atoi(argv[3]);
    y_end = atoi(argv[4]);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //calculating an appropriate endTheta:
    //
    endTheta = atan2((y_end - y_start),(x_end - x_start));

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
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    Dijkstra3D::Coordinate start(x_start,y_start,15);//mapTheta_end_correct);
    Dijkstra3D::Coordinate goal(x_end,y_end,15);//mapTheta_end_correct);
	AV_lce_Eval tunnelEval(vspace,goal,start);           // Heuristic eval, back to A*
	Dijkstra3D::CellEvaluator defaultEval; // Default eval, all cell cost are 0

    Dijkstra3D::TransitionApplicator defaultTrans;
    AV_TransApply cylinderTrans(vspace,headingTable16);
    //GAB:
    AV_ConnectEval connectXEval(headingTable16);

	Dijkstra3D::PathFinder<navi3dCell> pfinder(vspace, tunnelEval, transXEval, 
            cylinderTrans, connectXEval);
	//Dijkstra3D::PathFinder<navi3dCell> pfinder(vspace,connectivity8, defaultEval, trans8Eval, cylinderTrans);
	//Dijkstra3D::PathFinder<navi3dCell> pfinder(vspace,connectivity6, defaultEval, trans6Eval, cylinderTrans);
	pfinder.setExhaustiveSearch(false); // Do we want to stop at the first hit to the goal

	printf("Starting search\n");
	int res = pfinder.search(start, goal, path);

	printf("Search result %d Path size %d\n",res,path.size());
	Dijkstra3D::ShortestPath::const_iterator it;
	unsigned int i = 0;
    for (it=path.begin();it!=path.end();it++,i++) {
		printf("%d: %d %d %d\n",i,it->x,it->y,it->theta);
	}
    
#ifdef GNUPLOT_ENABLED

    FILE * search;
	FILE * pathfile;
    search = fopen("search","w");
    pathfile = fopen("path","w");
    fprintf(search,"%d %d %f %f \n",start.x,start.y,cos(headingTable16[start.theta]),sin(headingTable16[start.theta]));
    fprintf(search,"%d %d %f %f \n",goal.x,goal.y,cos(headingTable16[goal.theta]),sin(headingTable16[goal.theta]));
    fprintf(search,"%d %d %f %f \n",50,160,-20*cos(windDirection),-20*sin(windDirection));
    fclose(search);

    //Dijkstra3D::ShortestPath::const_iterator it;
    for(it=path.begin();it!=path.end();it++)
    {
        fprintf(pathfile,"%d %d \n",it->x,it->y);
    }
    fclose(pathfile);

    /**determination of x- and yrange: NUR POSITIVE ZAHLEN, SONST SEG FAULT**/
    int xmin = 0;
    int xmax = vspace.dimX().n;
    int ymin = 0;
    int ymax = vspace.dimY().n;
    gpl.setLogging(false);
    gpl.setEcho(false);
    gpl.plot ("set xrange [%d:%d]; set yrange [%d:%d]", xmin, xmax, ymin,ymax);
    //gpl.plot ("set terminal x11");
    gpl.plot ("set xlabel 'Latitude';set ylabel 'Longitude';set size ratio -1;set grid lt 6");

    /**save the path to a file .eps:**/
    //gpl.plot("set term postscript eps color blacktext 'Helvetica' 24; set output '2D_upperlakeside_w270.eps'");

    /**drawing the A*-generated path**/


    /**drawing the stuff:**/
    gpl.plot("plot 'path' with l lw 3, 'search' with vector notitle, 'gnuplot_islands' with points ps 1 pt 1 notitle");

#endif //GNUPLOT_ENABLED



    getchar ();
	return 0;

}
	
	

