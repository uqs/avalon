// General Things
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include "avalon.h"

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

double sailor_inverted_linear_model(double heading_speed, double torque_des, 
		double speed_x, double speed_y) 
{    
    double dens_water      = 1025.0;       
    double A_rudder        = 0.085;
    double v_r_tot;
    double d_water;
    double incid_angle;
    double rudder_angle;

    v_r_tot         = sqrt((speed_x*speed_x) + ((speed_y - 1.7*heading_speed)*(speed_y - 1.7*heading_speed)));
    d_water         = atan2((speed_y - 1.7*heading_speed),speed_x); 
    incid_angle = -torque_des/(0.9*1.7*dens_water*v_r_tot*v_r_tot*A_rudder);
    rudder_angle = remainder(incid_angle + d_water,2*AV_PI)*180/AV_PI;

    if(rudder_angle > 10)
    {
	return 10;
    }
    if(rudder_angle < -10)
    {
      return -10;
    }
	return rudder_angle;
}





