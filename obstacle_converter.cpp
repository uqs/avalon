#include "avalon.h"
/**general includes**/
#include <ctype.h>
#include <memory.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

//for setIslands
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <iostream>

//for debugging purposes
#define DEBUG_CONVERSION

void islandConversion(const char *island_pgm, FILE * newfile, 
        int xIsland1, int yIsland1,double xIslandLongitude1,double yIslandLatitude1,
        int xIsland2, int yIsland2,double xIslandLongitude2,double yIslandLatitude2)
{
    std::string line;
    std::ifstream myfile (island_pgm);
    int xSize,ySize,color;
    int xCount=0,yCount=0;
    double longitude1, latitude1;
    double longitude2, latitude2;
    double new_longitude, new_latitude;
    int nodemeters;

    longitude1 =double (AV_EARTHRADIUS 
        *cos((yIslandLatitude1 * AV_PI/180))*(AV_PI/180)
        *xIslandLongitude1);

    latitude1 =double (AV_EARTHRADIUS
        *(AV_PI/180)*yIslandLatitude1);

    longitude2 =double (AV_EARTHRADIUS 
        *cos((yIslandLatitude2 * AV_PI/180))*(AV_PI/180)
        *xIslandLongitude2);

    latitude2 =double (AV_EARTHRADIUS
        *(AV_PI/180)*yIslandLatitude2);

    nodemeters = (int)((longitude2 - longitude1)/(xIsland2 - xIsland1));
    //nodemeters = (latitude2 - latitude1)/(yIsland2 - yIsland1);

#ifdef DEBUG_CONVERSION
    printf("nodemeters = %d \n",nodemeters);
#endif

    char currentLine[10000], *ptr;
    if (myfile.is_open())
    {

        /**getting to know the size of the picture**/

        myfile.getline (currentLine,100,'\n');
        myfile.getline (currentLine,100,'\n');

        while(currentLine[0]=='#')
        {
            myfile.getline (currentLine,10,'\n');
        }

        //  std::cout << "current Line: " << currentLine << std::endl;
        xSize = strtol(currentLine,&ptr,0);
        ySize = atoi(ptr);
#ifdef DEBUG_CONVERSION
        printf("xSize is %d and ySize = %d \n ",xSize,ySize);
#endif

        myfile.getline (currentLine,10,'\n'); //to get to the max colour
        
        ///////////////////////////////////////////////////////////////////////////
        //write placement in the new file:
        //fprintf(newfile,"%f %f \n",xIslandLongitude1,yIslandLatitude1);
        //fprintf(newfile,"%d %d \n",(xIsland1*nodemeters/AV_NAVI_GRID_SIZE),((ySize- yIsland1)*nodemeters/AV_NAVI_GRID_SIZE));
        //fprintf(newfile,"################# \n");
        ///////////////////////////////////////////////////////////////////////////
        
        while (yCount<(ySize+1))
        {
            xCount = 0;
            while (xCount<xSize) //ga die einzelne Reihe in xrichtig düre
            {
                myfile.getline (currentLine,10000,'\n'); //to get tho the start of the image the first time!

                color = strtol(currentLine,&ptr,10);

                if (color==0)
                {               
                    new_longitude = longitude1 - nodemeters * (xIsland1 - xCount); 
                    new_latitude = latitude1 + nodemeters * (yIsland1 - yCount); 
                    //xi=(nodemeters/AV_NAVI_GRID_SIZE)*xCount + cx;
                    //yi=(nodemeters/AV_NAVI_GRID_SIZE)*ySize - (nodemeters/AV_NAVI_GRID_SIZE)*yCount - cy;
#ifdef DEBUG_CONVERSION
                    printf("xCount = %d yCount = %d \n",xCount,yCount);
#endif
                    //write to file:
                    fprintf(newfile,"%f %f\n",new_longitude, new_latitude);

                }

                xCount++;
            }
            yCount++;
        }

        fprintf(newfile,"################# \n"); // to mark the end!
        fprintf(newfile,"################# \n"); // to mark the end!
        fclose (newfile);
        myfile.close();
    }

else std::cout << "Unable to open file";
}


int main (int argc, char *argv[])
{

    //islandConversion(const char *island_pgm, FILE * newfile, 
    //        int xIsland1, int yIsland1,double xIslandLongitude1,double yIslandLatitude1, int xIsland2, int yIsland2,double xIslandLongitude2,double yIslandLatitude2)

    FILE * paper_island;
    paper_island = fopen ("seekarte","w+");
    islandConversion("zuerisee.pgm",paper_island,45,14,8.539745,47.36675, 178,310,8.5726233,47.3171316);

    return 0;
}
