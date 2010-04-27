
#ifndef SAILOR_RUDDER_ITER_FN_H
#define SAILOR_RUDDER_ITER_FN_H

// parameters must be in radian !!!!!!!!!!
//
// Uncomment the following line to active root finding and not minimsation
// #define ROOT_FINDING

struct rudder_iter_params
   {
       double heading_speed; 
       double torque_des; 
       double speed_x; 
       double speed_y;  
       double aoa_sail; 
       double d_wind; 
       double pose_3; 
       double V_wind;
   };

double sailor_rudder_iter_fn( double x, void *params );


int signum(double i); // gives back the sign of an int




#endif


