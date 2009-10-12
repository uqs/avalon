/**--------------start Oceanmpa2d.cpp--------------------**/


/**cpp file where all the functions of Oceanmap are defined and initialized**/

/**if defined, printing for debugging is enabled**/
//#define _DEBUG_COST
//#define _DEBUG_COST_2
//#define _DEBUG_COST_3
//#define _DEBUG_NEIGHBORHOOD
//#define _DEBUG_WIND
//#define _DEBUG_WIND_SPEED


/**general includes**/
#include <ctype.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include <vector>
#include <iostream>
#include "include/Grid2D.h"
#include "Oceanmap2d.h"

//for setIslands
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>

#include "Mapnode.h"
#include "micropather.h"
#include "avalon.h"

using namespace micropather;


void Oceanmap2d::SetNeighborhood8()
    {

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
                    neighbor neighborNow;
                    neighborNow.x =i;
                    neighborNow.y =j;

                    neighborNow.traversedCells.push_back(std::pair<int,int>(i,j));
                    neighborNow.traversedCells.push_back(std::pair<int,int>(0,0));
                    neighborhood.push_back (neighborNow);
                }
            }
        }
    }


void Oceanmap2d::SetNeighborhood24()
    {
        int z,traversedCellSize;
        double deltax, deltay;
        int currentX, currentY;

        for (int i=-2; i<3; i++)
        {
            for (int j=-2; j<3; j++)
            {
                #ifdef _DEBUG_NEIGHBORHOOD
                printf("i = %d, j=%d \n",i,j);
                #endif //debug neighborhood


                if ((i==0) && (j==0))
                {
                    continue;
                }
                else
                {
                    neighbor neighborNow;
                    neighborNow.x = i;
                    neighborNow.y = j;

                    deltax = 0;
                    deltay = 0;
                    z=0;

                    while (z<5)
                    {
                        deltax += ((double)i)/5;
                        deltay += ((double)j)/5;

                        currentX = rint(deltax);
                        currentY = rint(deltay);

#ifdef _DEBUG_NEIGHBORHOOD
printf("deltax und deltay: %f,%f; rounded: %d,%d;\n",deltax,deltay,currentX,currentY);
#endif


                        if (z==0)
                        {
                            neighborNow.traversedCells.push_back(std::pair<int,int>(currentX,currentY));
                            z++;
                        }
                        else
                        {
                            traversedCellSize = neighborNow.traversedCells.size() - 1;
                            z++;

                            if (neighborNow.traversedCells[traversedCellSize].first==currentX && neighborNow.traversedCells[traversedCellSize].second==currentY)
                            {
                                continue;
                            }
                            else
                            {
                                neighborNow.traversedCells.push_back(std::pair<int,int>(currentX,currentY));
                            }


                        }
                        #ifdef _DEBUG_NEIGHBORHOOD
                        printf("grösse des traversed Cells - Vector: %d\n",neighborNow.traversedCells.size());
                        #endif //debug neighborhood
                    }

                    neighborhood.push_back (neighborNow);
                }

            }
        }
    }




float Oceanmap2d::LeastCostEstimate (void *stateStart, void *stateEnd)
    {
        int dx = ((Mapnode2d *) stateStart)->x - ((Mapnode2d *) stateEnd)->x;
        int dy = ((Mapnode2d *) stateStart)->y - ((Mapnode2d *) stateEnd)->y;
        return (float) sqrt ((double) (dx * dx) + (double) (dy * dy));
    }



void Oceanmap2d::AdjacentCost (void *state,std::vector < micropather::StateCost > *adjacent)
    {
        /**for each of the surrounding nodes this routine calculates the cost**/
        Mapnode2d current = *(Mapnode2d *) state;
        int i,j,m,n;
        double currentCost=0,dist,headingAngle,alpha, currentSpeed;



        for (unsigned int k = 0; k<neighborhood.size();k++)
        {
            i= (signed)current.x + neighborhood[k].x;
            j= (signed)current.y + neighborhood[k].y;
            currentCost=0;

            for (unsigned int l=0;l<neighborhood[k].traversedCells.size ();l++)
            {
                m=neighborhood[k].traversedCells[l].first + current.x;
                n=neighborhood[k].traversedCells[l].second + current.y;
                if (map.contains(m,n))
                {

                    currentCost += map(m, n).cost;

                }
            }
            
            if (!map.contains(i,j))
            {
                continue;
            }
            else
            {
                /**good cell, use currentCost to avoid obstacles!**/

                //new version for calculating alpha:
                dist = sqrt (pow (i,2) + pow (j,2));

                headingAngle = atan2 (neighborhood[k].y, neighborhood[k].x);

                windDirection = remainder( windDirection, 2*AV_PI);
                alpha = fabs( remainder((headingAngle - windDirection),2*AV_PI));


                /**calculating the currentSpeed**/
                if (alpha<(AV_PI/4-0.05))
                {
                    currentSpeed=0.0;
                }
                else
                {
                currentSpeed = ((-0.00482179 * pow (alpha, 4) + 0.0335788 * pow (alpha, 3) - 0.246088 * pow (alpha,2)
                + 0.746539 * alpha - 0.0110976) * (double) windSpeed);
                }


                //printf("expanding from %d,%d to %d,%d with total cost of: %f \n",(signed)current.x,(signed)current.y,i,j,(currentCost + dist/currentSpeed));
                /**push back the newly calculated adjacent node**/
                if(currentSpeed==0)
                {
                    StateCost nodeCost = { (void *) &map (i, j), 20000};
                    adjacent->push_back (nodeCost);
                }
                else
                {
                    StateCost nodeCost = { (void *) &map (i, j), currentCost + dist/currentSpeed};
                    adjacent->push_back (nodeCost);
                }

            }
            assert (adjacent->size()>0);
        }

    }

void Oceanmap2d::PrintStateInfo (void *state)
    {
        ((Mapnode2d *) state)->print (stdout);
    }

void Oceanmap2d::Print (std::vector < void *>path)
    {
        for (unsigned int k = 0; k < path.size (); ++k)
        {
            ((Mapnode2d *) path[k])->print (stdout);
            printf ("vectorsize=%d\n",path.size());
        }
    }



Oceanmap2d::Oceanmap2d (unsigned int width, unsigned int height):map (0, 1, width, 0, 1, height) {
        Grid2D < Mapnode2d >::iterator it;
        for (it = map.begin (); it != map.end (); ++it)
        {
            it->x = map.xindex (it);
            it->y = map.yindex (it);

        }

        if(NEIGHBORHOOD == 8)
             SetNeighborhood8();
        if(NEIGHBORHOOD == 24)
             SetNeighborhood24();

    }

Oceanmap2d::~Oceanmap2d(){
}

void Oceanmap2d::setIslands (const char *island, int xIsland, int yIsland,int xIslandLongitude,int  yIslandLatitude, int nodemeters)
{
    std::string line;
    std::ifstream myfile (island);
    int xSize,ySize,xi,yi,color,cx,cy;
    int xCount=0,yCount=0;

    char currentLine[10000], *ptr;
//printf("helle\n");
    if (myfile.is_open())
    {
//printf("hello2\n");

        /**getting to know the size of the picture**/

        myfile.getline (currentLine,100,'\n');

        myfile.getline (currentLine,100,'\n');
        //myfile.getline (currentLine,100, '\n');
        //myfile.getline (currentLine,1000);

        while(currentLine[0]=='#')
        {
            myfile.getline (currentLine,10,'\n');
            //printf("hello3\n");
        }

        std::cout << "current Line: " << currentLine << std::endl;
        xSize = strtol(currentLine,&ptr,0);
        ySize = atoi(ptr);

        //printf("xSize is %d and ySize = %d \n ",xSize,ySize);

        myfile.getline (currentLine,10,'\n'); //to get to the max colour
        FILE * testfile;
        testfile = fopen ("testlog.txt","w+");

        /**start the reading part**/

        while (yCount<ySize)
        {



            xCount = 0;

            while (xCount<xSize) //ga die einzelne Reihe in xrichtig düre
            {
                //printf("newline\n");
                myfile.getline (currentLine,10000,'\n'); //to get tho the start of the image the first time!


                //if (xCount==0) //to read out the color of the specific point
                //{
                    color = strtol(currentLine,&ptr,10);
                    //printf("hello5a, color = %d\n",color);
                //}
//                else
//                {
//                    color = strtol(ptr,&ptr,10);
//                    printf("hello5b, color = %ld\n",color);
//                }

                if (color==0)
                {
                    for (cx=0;cx<(nodemeters/GRID_SIZE) ;cx++)
                    {

                        for (cy=0;cy<(nodemeters/GRID_SIZE) ;cy++)
                        {
                            xi=xIslandLongitude - (nodemeters/GRID_SIZE)*xIsland +(nodemeters/GRID_SIZE)*xCount + cx;
                            yi=yIslandLatitude + (nodemeters/GRID_SIZE)*yIsland -(nodemeters/GRID_SIZE)*yCount - cy;

                            if (map.contains(xi,yi))
                            {
                                //printf("black at %d,%d \n",xi,yi);
                        	    map(xi,yi).cost=1000;
                        	    fprintf(testfile, "%d %d\n", xi, yi);
                            }
                        }
                    }
                }

                xCount++;
            }
        	yCount++;

        }

        fclose (testfile);
        myfile.close();
    }

    else std::cout << "Unable to open file";
}




/**------------end Oceanmap2d.cpp------------------**/
