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



int main (int argc, char *argv[])
{

    double long_d, lat_d;
    double long_m, lat_m;

    long_m = atof(argv[1]);
    lat_m = atof(argv[2]);


    lat_d = lat_m / (AV_EARTHRADIUS * (AV_PI/180));
    long_d = long_m / (AV_EARTHRADIUS * cos((lat_d *AV_PI/180))*(AV_PI/180));
    
    std::cout << "longitude in degrees" << long_d << std::endl;
    std::cout << "latitude in degrees " << lat_d <<std::endl;

    return 0;
}
