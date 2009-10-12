#ifndef GRID_2D_H
#define GRID_2D_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "GridDimension.h"

#include <list>
#include <vector>
#include <map>

#define DEBUG_DIJKSTRA

/**
 * \class Grid2D
 * Representation of a generic 3D VoxelSpace, can store anything.
 * In order to be saved/loaded, the template parameter must have a load,and
 * save function. See VoxTypes.h for example of Voxels
 * **/
template <class C>
class Grid2D 
{
	protected:
		/**
		 * The pointer to the memory segment containing the data
		 * **/
		C * data;
		/**
		 * The VoxelSpace Dimensions
		 * **/
		GridDimension X,Y;
		/**
		 * The number of element in this VoxelSpace
		 * **/
		unsigned int nelem;
	public :
		/** 
		 * Returns dimension X
		 * **/
		const GridDimension & dimX()const  {return X;}
		/** 
		 * Returns dimension Y
		 * **/
		const GridDimension & dimY()const  {return Y;}

               
	public:
		
		/**
		 * Default constructor
		 * Not very useful, except if the Grid2D is to be read from file
		 * later-on
		 * **/
		Grid2D() :
			X(0,1,1), Y(0,1,1)
		{
			X.step = 1;
			Y.step = X.n * X.step;
			nelem = Y.n * Y.step;
			//data = (C*)(malloc(nelem * sizeof(C)));
			data  = new C[nelem];
			assert(data != NULL);
		}
		
		/**
		 * Interesting constructor
		 * Obvious argument isn't it?
		 * The step in all dimensions are updated.
		 * **/
		Grid2D(double _xmin,double _xmax,unsigned int _nx,
				double _ymin,double _ymax,unsigned int _ny) :
			X(_xmin,_xmax,_nx), Y(_ymin,_ymax,_ny)
		{
			X.step = 1;
			Y.step = X.n * X.step;
			printf("X: ");X.save(stdout);
			printf("Y: ");Y.save(stdout);
			nelem = Y.n * Y.step;
			//data = (C*)(malloc(nelem * sizeof(C)));
			data  = new C[nelem];
			assert(data != NULL);
			//memset(data,0,nelem * sizeof(C));

		}
		
		Grid2D(const GridDimension & _X, const GridDimension & _Y) :
			X(_X), Y(_Y)
		{
			X.step = 1;
			Y.step = X.n * X.step;
			printf("X: ");X.save(stdout);
			printf("Y: ");Y.save(stdout);
			nelem = Y.n * Y.step;
			//data = (C*)(malloc(nelem * sizeof(C)));
			data  = new C[nelem];
			assert(data != NULL);
			//memset(data,0,nelem * sizeof(C));

		}
		
		~Grid2D() 
		{
			//free(data);
			delete [] data;
		}
		
		/**
		 * Save to binary representation.
		 * Warning: this is endianness dependent
		 * **/
		bool savebin(const char * fname) const
		{
			double dval[9];
			unsigned int ival[6];
			char c='b';
			FILE * fp = fopen(fname,"w");
			if (fp == NULL) {
				return false;
			}
			if (fwrite(&c,sizeof(char),1,fp)!=1) {
				fclose(fp);
				return false;
			}

			if (!X.savebin(fp)) {fclose(fp);return false;} 
			if (!Y.savebin(fp)) {fclose(fp);return false;} 
			if (fwrite(data,sizeof(C),nelem,fp) != nelem) {
				fclose(fp);
				return false;
			}
			fclose(fp);
			return true;
		}

		/**
		 * Save to ascii representation, as long as the C::save function
		 * respect that...
		 * **/
		bool save(const char * fname) const
		{
			char c='a';
			FILE * fp = fopen(fname,"w");
			if (fp == NULL) {
				return false;
			}

			if (fwrite(&c,sizeof(char),1,fp)!=1) {
				fclose(fp);
				return false;
			}
			if (!X.save(fp)) {fclose(fp);return false;} 
			if (!Y.save(fp)) {fclose(fp);return false;} 
			unsigned int i;
			for (i=0;i<nelem;i++) {
				if (!data[i].save(fp)) {
					fclose(fp);
					return false;
				}
			}
			
			fclose(fp);
			return true;
		}
			
		
		/**
		 * Load from binary or ascii representation.
		 * Warning: binary representation is endianness dependent
		 * **/
		bool load(const char * fname) 
		{
			char c=0;
			FILE * fp = fopen(fname,"r");
			if (fp == NULL) return false;
			delete [] data;
			if (fread(&c,sizeof(unsigned char),1,fp) != 1) {
				fclose(fp);
				return false;
			}
			switch (c) {
				case 'a': /* alpha mode */
					if (!X.load(fp)) {fclose(fp);return false;}
					if (!Y.load(fp)) {fclose(fp);return false;}
					nelem = Y.n * Y.step;
					//data=(C*)(realloc(data,nelem*sizeof(C)));
					data = new C[nelem];

					unsigned int i;
					for (i=0;i<nelem;i++) {
						if (!data[i].load(fp)) {
							fclose(fp);
							return false;
						}
					}
					break;
				case 'b': /* binary mode */
					X.loadbin(fp); Y.loadbin(fp); 
					nelem = Y.n * Y.step;
					//data=(C*)(realloc(data,nelem*sizeof(C)));
					data = new C[nelem];
					if (fread(data,sizeof(C),nelem,fp) != nelem) {
						fclose(fp);
						return false;
					}
					break;
				default:
					return false;
			}
			fclose(fp);
			return true;
		}

		/**
		 * Print the voxel space. Useful only for small space and debug
		 * */
		void print(FILE * fp = stdout)
		{
			unsigned int ix,iy;
			for (iy = 0;iy<Y.n;iy++) {
				for (ix = 0;ix<X.n;ix++) {
					fprintf(fp,"%u %u ",ix,iy);
					data[ix*X.step+iy*Y.step].print(fp);
					fprintf(fp,"\n");
				}
				fprintf(fp,"\n");
			}
			fprintf(fp,"\n");
		}
		
		bool contains(int x, int y) {
			if (x < 0) return false;
			if (y < 0) return false;
			if (x >= (signed)X.n) return false;
			if (y >= (signed)Y.n) return false;
			return true;
		}

		/**
		 * Const accessor to object ix,iy
		 * **/
		const C & operator()(unsigned int ix, unsigned iy) const {
			assert(ix < X.n);
			assert(iy < Y.n);
			return data[ix*X.step+iy*Y.step];
		}
		
		/**
		 * Mutable accessor to object ix,iy
		 * **/
		C & operator()(unsigned int ix, unsigned iy) {
			assert(ix < X.n);
			assert(iy < Y.n);
			return data[ix*X.step+iy*Y.step];
		}

		void setto(const C & c) {
			unsigned int i;
			for (i=0;i<nelem;i++) data[i] = c;
		}

	public:
		/**
		 * \class Grid2D::iterator
		 * An iterator to move around in the VoxelSpace
		 * **/
		class iterator 
		{
			protected :
				C * ptr;
				/** Can only be created by the VoxelSpace (friend) **/
				iterator(C * p) {ptr = p;}
			public :
				iterator() {ptr=NULL;}
				void operator++() {++ptr;}
				const iterator & operator=(const iterator & it) {
					ptr = it.ptr;
					return *this;
				}
				bool operator==(const iterator & it) const {
					return it.ptr == ptr;
				}
				bool operator!=(const iterator & it) const {
					return it.ptr != ptr;
				}
				const C & operator*()const  {return *ptr;}
				const C* operator->()const  {return ptr;}
				C & operator*() {return *ptr;}
				C* operator->() {return ptr;}

				/** Debug **/
				void print() const {printf("It::ptr %p\n",ptr);}

				friend class Grid2D;
		};
	public :

		/**
		 * Iterator to the beginning of the space (xmin,ymin,xmax)
		 * **/
		iterator begin()const  {
			iterator itbegin;
			itbegin.ptr = data;
			return itbegin;
		}

		/**
		 * Iterator to the end of the space 
		 * Warning: has in STL, this iterator points outside the voxelspace
		 * memory bloc. Do not dereference it!
		 * **/
		iterator end()const  {
			iterator itend;
			itend.ptr = data + nelem;
			return itend;
		}

		/**
		 * Iterator to the centre of the space
		 * **/
		iterator centre()const  {
			iterator it;
			int ix = X.n / 2;
			int iy = Y.n / 2;
			it.ptr = data + ix * X.step + iy * Y.step;
			return it;
		}
		
		/**
		 * Iterator to a specific position in the space
		 * **/
		iterator icell(unsigned int ix, unsigned int iy)const  {
			iterator it;
			assert (ix < X.n); 
			assert (iy < Y.n); 
			it.ptr = data + ix * X.step + iy * Y.step;
			return it;
		}
		
		/**
		 * Iterator to a specific position in the space
		 * **/
		iterator cell(double x, double y)const  {
			iterator it;
			int ix = (int)round((x - X.min)/X.delta);
			int iy = (int)round((y - Y.min)/Y.delta);
			assert (ix >=0); assert (ix < (signed)X.n); 
			assert (iy >=0); assert (iy < (signed)Y.n); 
			it.ptr = data + ix * X.step + iy * Y.step;
			return it;
		}
		
		
		/** x index associated with an iterator **/
		unsigned int xindex(const iterator & it)const  {
			unsigned int pos = ((unsigned int)(it.ptr-data));
			return (pos % Y.step) / X.step;
		}

		/** y index associated with an iterator **/
		unsigned int yindex(const iterator & it)const  {
			unsigned int pos = ((unsigned int)(it.ptr-data));
			return pos / Y.step;
		}

			
		/** x value associated with an iterator **/
		double xvalue(const iterator & it)const  {
			return X.min + xindex(it)*X.delta;
		}
		/** y value associated with an iterator **/
		double yvalue(const iterator & it)const  {
			return Y.min + yindex(it)*Y.delta;
		}

		/** iterator with next value of y (yindex + 1), if possible **/
		iterator up(const iterator & it)const  {
			iterator res = it;
			if (yindex(it) > 0) {
				res.ptr -= Y.step;
			}
			return res;
		}
		
		/** iterator with previous value of y (yindex - 1), if possible **/
		iterator down(const iterator & it)const  {
			iterator res = it;
			if (yindex(it) < (Y.n - 1)) {
				res.ptr += Y.step;
			}
			return res;
		}
			
		/** iterator with previous value of x (xindex - 1), if possible **/
		iterator left(const iterator & it)const  {
			iterator res = it;
			if (xindex(it) > 0) {
				res.ptr -= X.step;
			}
			return res;
		}
		
		/** iterator with next value of x (xindex + 1), if possible **/
		iterator right(const iterator & it)const  {
			iterator res = it;
			if (xindex(it) < (X.n - 1)) {
				res.ptr += X.step;
			}
			return res;
		}
			
		/** 
		 * move iterator it of (dx,dy): this is an index displacement. 
		 * dimension where displacement is not possible are let unchanged
		 * **/
		iterator moveidx(const iterator & it, 
				int dx,int dy,int dz)const  {
			int x,y;
			x = xindex(it) + dx;
			y = yindex(it) + dy;
			iterator res = it;
			if ((x>=0) && (x < (signed)X.n)) res.ptr += dx*X.step;
			if ((y>=0) && (y < (signed)Y.n)) res.ptr += dy*Y.step;
			return res;
		}

};

namespace Dijkstra2D {

	//////////////////////////////////////////////////////////////////////////
	//
	// Dijkstra operations. 
	//
	//////////////////////////////////////////////////////////////////////////

    /**
     * GAB: so we have the information globally:
     * */
    double windDirection;
    double windSpeed;


	struct Coordinate {
		int x, y;
		Coordinate() {x=y=-1;}
		Coordinate(const Coordinate & c) : x(c.x), y(c.y) {}
		Coordinate(unsigned int _x, unsigned int _y) : x(_x), y(_y) {}
		Coordinate operator+(const Coordinate & c) {
			return Coordinate(x + c.x, y + c.y);
		}
		bool operator==(const Coordinate & c) {
			return (x==c.x) && (y==c.y);
		}
	};
	typedef std::vector<Coordinate> ConnectivityList;
	typedef std::vector<double> TransitionCost;
	typedef std::list<Coordinate> ShortestPath;

	void prepareConnectivity4(ConnectivityList & list) {
		list.clear();
		list.push_back(Coordinate(+1,0));
		list.push_back(Coordinate(-1,0));
		list.push_back(Coordinate(0,+1));
		list.push_back(Coordinate(0,-1));
	}

	void prepareConnectivity8(ConnectivityList & list) {
		list.clear();
		for (int i=-1;i<=+1;i++) {
			for (int j=-1;j<=+1;j++) {
				if (!i && !j) continue;
				list.push_back(Coordinate(i,j));
			}
		}
	}

	// to be overloaded
	struct CellEvaluator {
		virtual double operator()(const Coordinate &) const {
			return 0.0;
		}
	};

	struct TransitionEvaluator {
		virtual const TransitionCost & operator()(const Coordinate &, const ConnectivityList &) const = 0;
	};

    struct TransitionApplicator {
        virtual Coordinate operator()(const Coordinate & c1, const Coordinate & c2) const {
            return Coordinate(c1.x+c2.x,c1.y+c2.y);
        }
    };

	typedef std::multimap<double, Coordinate, std::less<double> > Heap;
	
    struct CellCost {
		double value;
		Coordinate predecessor;
		CellCost() {
			value = NAN;
			// predecessor is negative
		}
	};

	template <class C>
		class PathFinder {
			protected:
			const Grid2D<C> & graph;
			const ConnectivityList & connectivity;
			const CellEvaluator & cellEval;
			const TransitionEvaluator & transEval;
			const TransitionApplicator & transApply;
			bool stopAtFirt;
			public:

			PathFinder( const Grid2D<C> & _graph, const ConnectivityList & _connectivity,
					const CellEvaluator & _cellEval, const TransitionEvaluator & _transEval,
                    const TransitionApplicator & tapply) :
				graph(_graph), connectivity(_connectivity),
				cellEval(_cellEval), transEval(_transEval), transApply(tapply), stopAtFirt(true) {}

			void setExhaustiveSearch(bool exhaustive) {stopAtFirt = !exhaustive;}

			int search(const Coordinate & start, const Coordinate & goal, ShortestPath & path)
			{
				unsigned int numpop=0,maxsize=0;
				Heap heap;
				Grid2D<CellCost> cost(graph.dimX(),graph.dimY());
				cost(start.x,start.y).value = 0;
				path.clear();
				heap.insert(Heap::value_type(0,start));
				while (!heap.empty()) {
					unsigned int i;
					numpop ++;
					if (heap.size() > maxsize) {
						maxsize = heap.size();
					}

					typename Heap::iterator hit = heap.begin();
					heap.erase(hit);
					Coordinate thisCell = hit->second;
					double thisCost = hit->first;
#ifdef DEBUG_DIJKSTRA
					printf("Pop: %d %d: %f\n",thisCell.x,thisCell.y,thisCost);
#endif
					const TransitionCost & tcost = transEval(thisCell,connectivity);
					for (i=0;i<connectivity.size();i++) {
						Coordinate trans = transApply(thisCell, connectivity[i]);
#ifdef DEBUG_DIJKSTRA
						printf("Considering trans to %d %d: (%d %d) + (%d %d)\n",
								trans.x,trans.y,
								thisCell.x,thisCell.y,
								connectivity[i].x,connectivity[i].y);
#endif
						if (!cost.contains(trans.x,trans.y)) continue;
#ifdef DEBUG_DIJKSTRA
						printf("Feasible\n");
#endif
						typename Grid2D<CellCost>::iterator it = cost.icell(trans.x,trans.y);
						double oldcost = it->value;
						double newcost = thisCost + tcost[i] + cellEval(trans);
#ifdef DEBUG_DIJKSTRA
						printf("Value improvement: %f -> %f\n",oldcost, newcost);
#endif
						if (isnan(oldcost) || (newcost < oldcost)) {
#ifdef DEBUG_DIJKSTRA
							printf("Worth it\n");
#endif
							it->predecessor = thisCell;
							it->value = newcost;
							if (stopAtFirt && (trans == goal)) {
								heap.clear(); // to terminate the loop
								break;
							} else {
								heap.insert(Heap::value_type(newcost,trans));
							}
						}
					}
				}
				printf("searchPath: %d pop, heap size up to %d\n",numpop,maxsize);
				if (isnan(cost(goal.x,goal.y).value)) {
					// No path was found
					return -1;
				}

				Coordinate cit(goal);
				do {
					path.push_front(cit);
					cit = cost(cit.x,cit.y).predecessor;
				} while ((cit.x >= 0) && (cit.y >= 0));

				return 0;
			}
		};


};
#endif // GRID_2D_H

