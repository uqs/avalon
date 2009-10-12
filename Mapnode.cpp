
#include "micropather.h"
#include "Mapnode.h"
//#include "Oceanmap2d.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

using namespace std;

/**mapndoe for the 2D-Grid**/

Mapnode2d::Mapnode2d ()
{
	x = 0;
	y = 0;
	cost = 0.0;
	parent = NULL;
}
Mapnode2d::Mapnode2d (const Mapnode2d & v)
{
	x = v.x;
	y = v.y;
	cost = v.cost;
	parent = NULL;
}




void Mapnode2d::load (FILE * fp)
{
	int r;
	r = fread (&x, sizeof (unsigned int), 1, fp);
	r = fread (&y, sizeof (unsigned int), 1, fp);
	r = fread (&cost, sizeof (float), 1, fp);
	parent = NULL;
}

void Mapnode2d::save (FILE * fp) const
{
	int r;
	r = fwrite (&x, sizeof (unsigned int), 1, fp);
	r = fwrite (&y, sizeof (unsigned int), 1, fp);
	r = fwrite (&cost, sizeof (float), 1, fp);
}

void Mapnode2d::print (FILE * fp) const
{
	fprintf (fp, "%d %d %f ", x, y, cost);
}


/**mapnode for the 3D-Grid**/


Mapnode3d::Mapnode3d ()
{
	x = 0;
	y = 0;
	theta = 0.0;
    //checkValue = 140;
	cost = 0.0;
	parent = NULL;
}
Mapnode3d::Mapnode3d (const Mapnode3d & v)
{
	x = v.x;
	y = v.y;
	theta = v.theta;
    //checkValue = v.checkValue;
	cost = v.cost;
	parent = NULL;
}


void Mapnode3d::load (FILE * fp)
{
	int r;
	r = fread (&x, sizeof (unsigned int), 1, fp);
	r = fread (&y, sizeof (unsigned int), 1, fp);
	r = fread (&theta, sizeof (double), 1, fp);
	//r = fread (&checkValue, sizeof (int), 1, fp);
	r = fread (&cost, sizeof (float), 1, fp);
	parent = NULL;
}

void Mapnode3d::save (FILE * fp) const
{
	int r;
	r = fwrite (&x, sizeof (unsigned int), 1, fp);
	r = fwrite (&y, sizeof (unsigned int), 1, fp);
	r = fwrite (&theta, sizeof (double), 1, fp);
	//r = fwrite (&checkValue, sizeof (int), 1, fp);
	r = fwrite (&cost, sizeof (float), 1, fp);
}

void Mapnode3d::print (FILE * fp) const
{
	fprintf (fp, "%d %d %f ", x, y, cost);
}

