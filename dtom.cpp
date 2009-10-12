#include "../ssa/avalon.h"
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



int main (int argc, char *argv[])
{

    double long_d, lat_d;
    double long_m, lat_m;

    long_d = atof(argv[1]);
    lat_d = atof(argv[2]);

    long_m =double (AV_EARTHRADIUS 
            *cos((lat_d * AV_PI/180))*(AV_PI/180)
            *long_d);
    lat_m =double (AV_EARTHRADIUS
            *(AV_PI/180)*lat_d);

    std::cout << "longitude in meters:" << long_m << std::endl;
    std::cout << "latitude in meters: " << lat_m <<std::endl;

    return 0;
}
