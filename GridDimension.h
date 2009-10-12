#ifndef GRID_DIMENSION_H
#define GRID_DIMENSION_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/**
 * \class GridDimension:
 * The description of a dimension, to be used in the Grid3D definition
 * A dimension is defined with a min, a max and a discretization step
 * This implies a number of *intervals* n.
 * When used by the Grid3D, the step parameter is filled: it is the number of
 * voxelspace between two values in this dimension
 * **/
struct GridDimension {
	double min,max,delta;
	unsigned int n,step;
	GridDimension(double _min,double _max,unsigned int _n) {
		min = _min;
		max = _max;
		n = _n;
		delta = (max - min)/n;
		step = 1;
	}
	
	GridDimension(const GridDimension & gd) {
		min = gd.min;
		max = gd.max;
		n = gd.n;
		delta = (max - min)/n;
		step = 1;
	}

	/**
	 * Save to an ascii representation
	 * **/
	bool save(FILE * fp) const {
		return fprintf(fp,"%f %f %f %u %u\n",min,max,delta,n,step) == 5;
	}
	/**
	 * Save to a binary representation.
	 * Warning: this is endianness dependent!
	 * **/
	bool savebin(FILE * fp) const {
		double d[3] = {min,max,delta};
		unsigned int i[2] = {n,step};
		if (fwrite(d,sizeof(double),3,fp) != 3) {
			return false;
		}
		if (fwrite(i,sizeof(unsigned int),2,fp) != 2) {
			return false;
		}
		return true;
	}
	/**
	 * Load from an ascii representation
	 * **/
	bool load(FILE * fp) {
		return fscanf(fp,"%le %le %le %u %u\n",&min,&max,&delta,&n,&step) == 5;
	}
	/**
	 * Load from a binary representation.
	 * Warning: this is endianness dependent!
	 * **/
	bool loadbin(FILE * fp) {
		double d[3] = {0,0,0};
		unsigned int i[2] = {0,0};
		if (fread(d,sizeof(double),3,fp)!=3) {
			return false;
		}
		if (fread(i,sizeof(unsigned int),2,fp)!=2) {
			return false;
		}
		min = d[0]; max = d[1]; delta = d[2];
		n = i[0]; step = i[1];
		return true;
	}

	/**
	 * \class GridDimension::iterator
	 * An interator to move on the dimension
	 * **/
	class iterator {
		protected :
			double min,max,dv,v;
		public :
			iterator(const GridDimension & d)  
			{
				min = d.min; max = d.max;
				dv = d.delta; v = min;
			}
			iterator(const iterator & it) {
				min = it.min; max = it.max;
				dv = it.dv; v = it.v;
			}
			iterator(const GridDimension & d,double val)  
			{
				min = d.min; max = d.max;
				dv = d.delta; v = val;
				if (v<min) v = min;
				if (v>max) v = max;
			}
			~iterator() {}
			const iterator & operator++() {v += dv; if (v>max) v = max; return *this;}
			const iterator & operator--() {v -= dv; if (v<min) v = min; return *this;}
			bool operator<(const iterator & it) {return v < it.v;}
			bool operator<=(const iterator & it) {return v <= it.v;}
			double operator*() {return v;}
			iterator end() {iterator it(*this);it.v = max;return it;}
	};
	iterator begin() const {return iterator(*this);}
	iterator end() const {return iterator(*this,max);}

	/**
	 * Conversion function: returns the value corresponding to an index, 
	 * that is the beginning of the i-th interval
	 * **/
	double value(unsigned int i) const {
		if (i<n) return min + i*delta;
		return max;
	}
	/**
	 * Conversion function: return the index of the interval containing v
	 * **/
	unsigned int index(double v) const {
		if (v < min) return 0;
		unsigned int r = (unsigned int)floor((v-min)/delta);
		if (r >= n) return n-1;
		return r;
	}
};


#endif // GRID_DIMENSION_H
