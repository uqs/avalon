/**-------------start of Mapnode2d.cpp------------------**/

#ifndef MAPNODE_H
#define MAPNODE_H

#include <stdio.h>

struct Mapnode2d
{
	unsigned int x, y;
	float cost;
	Mapnode2d *parent;


	Mapnode2d();

	Mapnode2d (const Mapnode2d & v);

	const Mapnode2d & operator= (const Mapnode2d & v)
	{
		x = v.x;
		y = v.y;
		cost = v.cost;
		parent = NULL;
		return *this;
	}

	void load (FILE * fp);

	void save (FILE * fp) const;

	void print (FILE * fp) const;

};

struct Mapnode3d
{
	unsigned int x, y;
	int theta; //mathematically correct degrees (east is 0 degrees)
    int checkValue;
	float cost;
	Mapnode3d *parent;


	Mapnode3d();

	Mapnode3d (const Mapnode3d & v);

	const Mapnode3d & operator= (const Mapnode3d & v)
	{
		x = v.x;
		y = v.y;
		theta = v.theta;
        checkValue = v.checkValue;
		cost = v.cost;
		parent = NULL;
		return *this;
	}

	void load (FILE * fp);

	void save (FILE * fp) const;

	void print (FILE * fp) const;

};

#endif //mapnode2d.h

/**---------end of Mapnode2d.h -----------------**/
