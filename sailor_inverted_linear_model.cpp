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
    double dist_rudder_CenterOfRotation = 1.7;
    double linearisation_coeff = 0.9;

    double v_r_tot;
    double d_water;
    double incid_angle;
    double rudder_angle;

    v_r_tot	= sqrt((speed_x*speed_x) + ((speed_y - dist_rudder_CenterOfRotation*heading_speed)*(speed_y - dist_rudder_CenterOfRotation*heading_speed)));
    d_water     = atan2((speed_y - dist_rudder_CenterOfRotation*heading_speed),speed_x)*180/AV_PI; 
    incid_angle = -torque_des / (linearisation_coeff*dist_rudder_CenterOfRotation*dens_water*v_r_tot*v_r_tot*A_rudder) *180/AV_PI;

    if(incid_angle > 10)
    {
	incid_angle = 10;
    }
    else if(incid_angle < -10)
    {
      incid_angle = -10;
    }
    rudder_angle = remainder(incid_angle + d_water,360.);


	return rudder_angle;
}





