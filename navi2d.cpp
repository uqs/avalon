/**
 * main function to call the 2D navigation algorithm
 * */


#include "Grid2D.h"
#include "GridTypes.h"
#include "movements2d.h"


int main(int argc,char * argv[]) 
{
	UISpace vspace(-1.3,1.3,2000,
			-M_PI,M_PI,2000);

	//to make it globally available:
    Dijkstra2D::windDirection = 10.0; 
    

    if (argc > 1) {
		if (!vspace.load(argv[1])) {
			printf("Error while loading file %s\n",argv[1]);
		} else {
			printf("Loaded file %s\n",argv[1]);
		}
	} else {
		UISpace::iterator it;
		for (it = vspace.begin();it != vspace.end(); ++it) {
			it->value = 0x00;
			unsigned int xi =  vspace.xindex(it);
			if ((xi % 2) != 0) continue;
			unsigned int yi =  vspace.yindex(it);
			if ((yi % 2) != 0) continue;
			(*it).value = 0xFF;
		}
		// Direct cell access using cell index
		vspace(0,0) = 38;
		// Direct cell access using the cell index and iterator
		it = vspace.icell(3,1);
		it->value = 45;
		// Direct cell access using the cell coordinates
		it = vspace.cell(-0.2,M_PI/4);
		it->value = 42;

		const char * fname = "test.v2d";
		if (!vspace.save(fname)) {
			printf("Error while saving file %s\n",fname);
		} else {
			printf("Saved file %s\n",fname);
		}
	}

	// vspace.print();

	Dijkstra2D::ShortestPath path;
	Dijkstra2D::ConnectivityList connectivity4;
	Dijkstra2D::ConnectivityList connectivity8;
	Dijkstra2D::prepareConnectivity4(connectivity4);
	Dijkstra2D::prepareConnectivity8(connectivity8);
	UITEval uit4Eval(connectivity4); // Evaluate the cost of a 4 connectivity
	UITEval uit8Eval(connectivity8); // Evaluate the cost of a 8 connectivity

	Dijkstra2D::Coordinate goal(1,1990);
	UICEval heuristicEval(goal);           // Heuristic eval, back to A*
	Dijkstra2D::CellEvaluator defaultEval; // Default eval, all cell cost are 0

    Dijkstra2D::TransitionApplicator defaultTrans;
    UITApply cylinderTrans(vspace);

	Dijkstra2D::PathFinder<UICell> pfinder(vspace,connectivity8, defaultEval, uit8Eval, cylinderTrans);
	pfinder.setExhaustiveSearch(true); // Do we want to stop at the first hit to the goal

	printf("Starting search\n");
	int res = pfinder.search(Dijkstra2D::Coordinate(0,0), goal, path);

	printf("Search result %d Path size %d\n",res,path.size());
	Dijkstra2D::ShortestPath::const_iterator it;
	unsigned int i = 0;
	for (it=path.begin();it!=path.end();it++,i++) {
		printf("%d: %d %d\n",i,it->x,it->y);
	}

	return 0;
}
	
	

