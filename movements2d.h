/**
 * start of movements2d.h
 * where all the transitions for the 2d-navigations are called:
 * */

#ifndef MOVEMENTS2D_H
#define MOVEMENTS3D_H


#include "Grid2D.h"
#include "GridTypes.h"

struct UICell : public GenericCell<unsigned int> 
{
	UICell() : GenericCell<unsigned int>() {}
	UICell(const unsigned int & v) : GenericCell<unsigned int>(v) {}
#ifdef BINARY
	bool load(FILE * fp) {return fread(&value,sizeof(unsigned int),1,fp)==1;}
	bool save(FILE * fp) const {return fwrite(&value,sizeof(unsigned int),1,fp)==1;}
#else
	bool load(FILE * fp) { return fscanf(fp," %u ",&value) == 1; }
	bool save(FILE * fp) const { return fprintf(fp,"%u ",value) == 1;}
#endif
	void print(FILE * fp) const {fprintf(fp,"%u ",value);}

	
};

#define SQR(X) ((X)*(X))
//typedef Grid2D< IntegerCell<unsigned char> > UISpace;
typedef Grid2D< UICell > UISpace;


struct UITApply : public Dijkstra2D::TransitionApplicator
{
    const UISpace & grid;
    UITApply(const UISpace & g) : grid(g) {}

    virtual Dijkstra2D::Coordinate operator()(const Dijkstra2D::Coordinate & c1, 
            const Dijkstra2D::Coordinate & c2) const {
        int newy = c1.y+c2.y;
        int n = grid.dimY().n;
        //printf("newy %d\t",newy);
        while (newy >= n)
            newy -= n;
        while (newy < 0)
            newy += n;
        // printf("newy %d \n",newy);
        return Dijkstra2D::Coordinate(c1.x+c2.x,newy);
    }
};
struct UITEval : public Dijkstra2D::TransitionEvaluator
{
	Dijkstra2D::TransitionCost tc;
	UITEval(const Dijkstra2D::ConnectivityList & connectivity) {
		tc.resize(connectivity.size());
		for (unsigned int i=0;i<tc.size();i++) {
			tc[i] = sqrt(SQR(connectivity[i].x) + SQR(connectivity[i].y));
		}
	}

	virtual const Dijkstra2D::TransitionCost & operator()(const Dijkstra2D::Coordinate &, const Dijkstra2D::ConnectivityList &) const
	{
		return tc;
	}

};

struct UICEval : public Dijkstra2D::CellEvaluator 
{
	Dijkstra2D::Coordinate goal;
	UICEval(const Dijkstra2D::Coordinate & g) : goal(g) {}
	virtual double operator()(const Dijkstra2D::Coordinate & c) const {
		return sqrt(SQR(c.x-goal.x)+SQR(c.y-goal.y));
	}

};

#endif //ifndef

