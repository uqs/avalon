/**
 * defines all the transitions for the 3D-path planner
 * */

#include "avalon.h"
#include "Grid3D.h"
#include "GridTypes.h"
#include <math.h>

//#define DEBUG_CONNECTEVAL
//#define DEBUG_TRANSEVAL

#define SQR(X) ((X)*(X))
struct navi3dCell : public GenericCell<unsigned int> 
{
	navi3dCell() : GenericCell<unsigned int>(0) {}
	navi3dCell(const unsigned int & v) : GenericCell<unsigned int>(v) {}
#ifdef BINARY
    bool load(FILE * fp) {return fread(&value,sizeof(unsigned int),1,fp)==1;}
    bool save(FILE * fp) const {return fwrite(&value,sizeof(unsigned int),1,fp)==1;}
#else
    bool load(FILE * fp) { return fscanf(fp," %u ",&value) == 1; }
    bool save(FILE * fp) const { return fprintf(fp,"%u ",value) == 1;}
#endif
    void print(FILE * fp) const {fprintf(fp,"%u ",value);}

};

//to be of use in navi3d and movements.cpp
struct Neighbor {
    int x,y;
    std::vector<std::pair<int,int> > traversedCells; 
};

//typedef Grid3D< IntegerCell<unsigned char> > UISpace;
typedef Grid3D< navi3dCell > UISpace;

/** /////////////////////////////////////////////////////////////
 * Transition applicator, makes things with the connectivity-list
 * */////////////////////////////////////////////////////////////

struct AV_TransApply : public Dijkstra3D::TransitionApplicator
{
    const UISpace & grid;
    const std::vector<double> & headingTable;

    AV_TransApply(const UISpace & g, const std::vector<double>& headings) : grid(g),headingTable(headings) {}

    virtual Dijkstra3D::Coordinate operator()(const Dijkstra3D::Coordinate & c1, 
            const Dijkstra3D::Coordinate & c2) const {
          return Dijkstra3D::Coordinate(c1.x+c2.x,c1.y+c2.y,c2.theta);
    }
};

typedef std::vector<double> Alpha;
typedef std::vector<double> CurrentSpeed;
typedef std::vector<double> T_cost;

/**
 * Connectivity evaluator
 * */

struct AV_ConnectEval : public Dijkstra3D::ConnectivityEvaluator
{
    const std::vector<double> & headingTable;
    Dijkstra3D::ConnectivityList connect;
    
    AV_ConnectEval(const std::vector<double>& headings):headingTable(headings)  {}

    virtual Dijkstra3D::ConnectivityList & operator()
        (const Dijkstra3D::Coordinate & currPos)
    {
        double currHeading = headingTable[currPos.theta];
        int newTheta;
        double workTheta;
        int u,v,w;

        connect.clear();
        for(u=-2; u<3; u++)
        {
            for(v=-2; v<3; v++)
            {
                workTheta = atan2(v,u);
                if(u==0 && v==0) continue;
                if(fabs(remainder(currHeading - workTheta,2*AV_PI)) < 2*AV_PI/3)
                {
                    for(w=0; w<17; w++)
                    {
                        assert(w<16);
#ifdef DEBUG_CONNECTEVAL
                        printf("headingTable[%d] = %f; workTheta = %f \n",w,headingTable[w],workTheta);
#endif
                        if(fabs(remainder(headingTable[w] - workTheta,2*AV_PI)) < 1e-6 )
                        {
                            newTheta = w;
#ifdef DEBUG_CONNECTEVAL
                            printf("connectivity: dx = %d, dy = %d, -> theta = %d \n",u,v,w);
#endif
                            break;
                        }
                    }
#ifdef DEBUG_CONNECTEVAL
                    printf("connectivity-for schleife goes through till w = %d \n",w);
#endif
                    connect.push_back(Dijkstra3D::Coordinate(u,v,newTheta));
                }
            }
        }

#ifdef DEBUG_CONNECTEVAL
        printf("size of connectivity: %d \n",connect.size());
#endif

#if 0
        //always be able to turn on the spot:
        connect.push_back(Dijkstra3D::Coordinate(0,0,+1));
        connect.push_back(Dijkstra3D::Coordinate(0,0,-1));
		//additional moves depend on current heading:
        switch(currPos.theta)
        {
            case 0:
                connect.push_back(Dijkstra3D::Coordinate(+1,0,0));
                connect.push_back(Dijkstra3D::Coordinate(2,2,+1));
                connect.push_back(Dijkstra3D::Coordinate(+2,-2,-1));
                break;
            case 1:
                connect.push_back(Dijkstra3D::Coordinate(0,3,+1));
                connect.push_back(Dijkstra3D::Coordinate(1,1,0));
                connect.push_back(Dijkstra3D::Coordinate(3,0,-1));
                break;
            case 2:
                connect.push_back(Dijkstra3D::Coordinate(0,1,0));
                connect.push_back(Dijkstra3D::Coordinate(-2,+2,+1));
                connect.push_back(Dijkstra3D::Coordinate(+2,+2,-1));
                break;
            case 3:
                connect.push_back(Dijkstra3D::Coordinate(-1,1,0));
                connect.push_back(Dijkstra3D::Coordinate(-3,0,+1));
                connect.push_back(Dijkstra3D::Coordinate(0,3,-1));
                break;
            case 4:
                connect.push_back(Dijkstra3D::Coordinate(-1,0,0));
                connect.push_back(Dijkstra3D::Coordinate(-2,-2,+1));
                connect.push_back(Dijkstra3D::Coordinate(-2,+2,-1));
                break;
            case 5:
                connect.push_back(Dijkstra3D::Coordinate(-1,-1,0));
                connect.push_back(Dijkstra3D::Coordinate(0,-3,+1));
                connect.push_back(Dijkstra3D::Coordinate(-3,0,-1));
                break;
            case 6:
                connect.push_back(Dijkstra3D::Coordinate(0,-1,0));
                connect.push_back(Dijkstra3D::Coordinate(2,-2,+1));
                connect.push_back(Dijkstra3D::Coordinate(-2,-2,-1));
                break;
            case 7:
                connect.push_back(Dijkstra3D::Coordinate(+1,-1,0));
                connect.push_back(Dijkstra3D::Coordinate(3,0,+1));
                connect.push_back(Dijkstra3D::Coordinate(0,-3,-1));
                break;
        }
#endif
        return connect;
    }
};
    
/** ///////////////////////////////////////////////////////////////
 * Transition evaluator
 * *//////////////////////////////////////////////////////////////

struct AV_TransEval : public Dijkstra3D::TransitionEvaluator
{
	Dijkstra3D::TransitionCost t_dist; //in meters
    Dijkstra3D::TransitionCost totalCost;
    double windSpeed; 
    double windDirection;
    int k;
    int z;
    double deltax, deltay;
    int currentX, currentY;


    Alpha alpha;
    std::vector<double> currentSpeed; //in knots
    std::vector<double> t_cost;
    std::vector<double> turning_cost;

    std::vector<std::pair<int,int> > traversedCells;
    const std::vector<double> & headingTable;

	//AV_TransEval(const Dijkstra3D::ConnectivityList & connectivity, const std::vector<double> headings, 
    //         double windSpeed, double windDirection);
	AV_TransEval(const std::vector<double>& headings, 
            double _windSpeed, double _windDirection) : headingTable(headings) {
    windSpeed = _windSpeed;
    windDirection = _windDirection;

    };

	virtual const Dijkstra3D::TransitionCost & operator()
        (const Dijkstra3D::Coordinate & c, const Dijkstra3D::ConnectivityList & list) 
	{
        t_dist.resize(list.size());
        alpha.resize(list.size());
        t_cost.resize(list.size());
        currentSpeed.resize(list.size());
        totalCost.resize(list.size());
        turning_cost.resize(list.size());

#ifdef DEBUG_TRANSEVAL
        printf("check: windDirection = %f, windSpeed = %f \n",windDirection,windSpeed);
#endif

        for (unsigned int i=0;i<t_dist.size();i++) {
            t_dist[i] = AV_NAVI_GRID_SIZE*sqrt(SQR(list[i].x) + SQR(list[i].y)); //in m!
            alpha[i] = fabs(remainder((headingTable[list[i].theta] - windDirection),2*AV_PI)); //modified for new connect eval

            /////////////////////////////////////
            /**calculating the currentSpeed**/
            if (alpha[i]<(M_PI/4-0.05) || alpha[i] > 155*AV_PI/180)
            {
                currentSpeed[i]=0.0;
            }
            else
            {
                currentSpeed[i] = ((-0.00482179 * pow (alpha[i], 4) + 0.0335788 * pow (alpha[i], 3) - 0.246088*alpha[i]*alpha[i] 
                            + 0.746539 * alpha[i] - 0.0110976) * windSpeed)*0.514444; //in m/s!!
            }

#ifdef DEBUG_TRANSEVAL
            printf("alpha[%d] = %f -> currentSpeed = %f \n ",i,alpha[i],currentSpeed[i]);
#endif
            /////////////////////////////////////
            //turning cost:
            turning_cost[i] = 12.0 * fabs(remainder((headingTable[list[i].theta] - headingTable[c.theta]),2*AV_PI)); 
#if 0
            /////////////////////////////////////
            //cost for tack / jibe:
            if (((headingTable[c.theta] - windDirection)*(headingTable[list[i].theta] - windDirection) < 0))
            {
                turning_cost[i] += 0.0; //25.0;
            }
#endif
            /////////////////////////////////////
            //add the total cost together
            if(fabs(currentSpeed[i]) < 1e-5)
            {
                t_cost[i] = 80000; //make sure not to go there!
            }
            else
            {
                t_cost[i] = t_dist[i]/currentSpeed[i] + turning_cost[i];
                assert(t_cost[i]>=0);
#ifdef DEBUG_TRANSEVAL
                printf("t_cost[%d](dist/speed) = %f \n",i,t_cost[i]);
#endif

            }
        }
        return (t_cost);
	}

};

/**
 * Cell Evaluator, contains LeastCostEstimate & currentCost (island-passing)
 * */

struct AV_lce_Eval : public Dijkstra3D::CellEvaluator 
{
    UISpace & oceanmap;
	Dijkstra3D::Coordinate goal;
	Dijkstra3D::Coordinate start;
    std::vector<Neighbor> neighborhood;
	int vec_sg_x,vec_sg_y;
    int vec_currg_x, vec_currg_y;
    double dist_soll;
    double dist_goal;
    double tunnel_cost;

    AV_lce_Eval(UISpace & map,const Dijkstra3D::Coordinate & g, const Dijkstra3D::Coordinate & s) : 
        oceanmap(map),goal(g),start(s) {}
	
    
    virtual double operator()(const Dijkstra3D::Coordinate & c) {

        // tunnel cost:	
        vec_sg_x = goal.x - start.x;
        vec_sg_y = goal.y - start.y;

        vec_currg_x = goal.x - c.x;
        vec_currg_y = goal.y - c.y;

        dist_soll = fabs(((vec_sg_x * vec_currg_y) - (vec_sg_y * vec_currg_x))/
                (sqrt(vec_sg_x*vec_sg_x + vec_sg_y*vec_sg_y)))*AV_NAVI_GRID_SIZE;

        tunnel_cost = 0.0;
        //if(dist_soll*AV_NAVI_GRID_SIZE/AV_LAKE_TUNNEL > 0.6)
        {
          //  tunnel_cost = fabs(dist_soll*AV_NAVI_GRID_SIZE/AV_LAKE_TUNNEL)*2;
        }
        //if(dist_soll*AV_NAVI_GRID_SIZE > AV_LAKE_TUNNEL) tunnel_cost = 0;
        //tunnel_cost = fabs(dist_soll*AV_NAVI_GRID_SIZE/AV_LAKE_TUNNEL)*2;

#ifdef DEBUG_LCE
        printf("dist_soll = %f, -> tunnel_cost = %f \n",dist_soll,tunnel_cost);
#endif

        //heuristic: (moving 1 knot from current to goal)
        dist_goal =0.0;//AV_NAVI_GRID_SIZE*sqrt(vec_currg_x*vec_currg_x + vec_currg_y*vec_currg_y)/0.5144444;

        return((oceanmap(c.x,c.y,0).value + tunnel_cost + dist_goal));
    }

};



	
	

