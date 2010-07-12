// General Things
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include "avalon.h"
#include "sailor_rudder_iter_fn.h"
// #include "sailor_rudder_int_fn.h"

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

double sailor_rudder_iter_fn(double x, void *params) 
{    
    struct rudder_iter_params *p
          = (struct rudder_iter_params *) params;

    double heading_speed   = p->heading_speed;
    double torque_des      = p->torque_des;
    double speed_x         = p->speed_x;
    double speed_y         = p->speed_y;
    double aoa_sail        = p->aoa_sail;
    double d_wind          = p->d_wind;
    double pose_3          = p->pose_3;
    double V_wind          = p->V_wind;

    //double delta_t         = 0.2;
    double dens_air        = 1.23;             
    double dens_water      = 1025.0;             
    //double g               = 9.81;              

    double A_hull_3        = 2.0;            
    double C_d_3           = 1.3;        
    double width           = 1.2;
    double A_sail          = 8.4;
    double A_rudder        = 0.085;
    // double I_3             = 150;
    // double dist_to_rud     = 1.7;

    double N_damping;
    double N_damp_rot;
    double N_sail;
    double N_des;
    double N_rudder_des;
    double Y_rudder;
    double Y_rudder_right;
    double k;
    double v_r_tot;
    double d_water;
    double incid_angle;
    double vorzeichenR;
    double vorzeichenRR;
    // double y;
    double aoa;
    double d_wind_r;
    double d2aoa;
    double c_sail_lift;
    double c_sail_drag;
    double F_lift_V_w;
    double F_drag_V_w;
    double vorzeichen;
    double x_to_sail_coa;
    double y_to_sail_coa;
    double X_sail;
    double Y_sail;
    double sail_factor;
    double c_rudder_lift;
    double c_rudder_drag;
    double F_lift_v_right;
    double F_drag_v_right;
    double F_lift_v_left;
    double F_drag_v_left;
    double V_wind_x;
    double V_wind_y;
    
    // neccessary coordn. transformation
//     speed_y              = -speed_y;

V_wind_x=(V_wind*cos(d_wind-pose_3)+speed_x);
V_wind_y=(V_wind*sin(d_wind-pose_3)+speed_y);
V_wind=sqrt((V_wind_x)*(V_wind_x)+(V_wind_y)*(V_wind_y));
// double d_wind_1=remainder(atan2(V_wind_y,V_wind_x)-aoa_sail,2*AV_PI);
// rtx_message("Wind: Speed= %f, angle= %f",V_wind, d_wind);
//     torque_des           = 0.7*torque_des;
    N_damping            = signum(heading_speed)*A_hull_3*0.5*dens_water*C_d_3*((width/2.0*heading_speed)*(width/2.0*heading_speed));
    N_damp_rot           = signum(heading_speed)*1600.0*(width/2.0*heading_speed*heading_speed);
    N_damping            = N_damping + N_damp_rot;

    aoa                  = aoa_sail;  // [rad]
    d_wind_r             = d_wind - pose_3;  // [rad] 
    d_wind_r             = remainder((d_wind_r*180.0/AV_PI),360.0)*AV_PI/180.0;

    d2aoa                = remainder(((-d_wind_r+aoa)*180.0/AV_PI),360.0)*AV_PI/180.0;
// rtx_message("aoa: %f  d2aoa: %f  d_wind_r: %f",aoa, d2aoa, d_wind_r);

    if (fabs(d2aoa) <= 2.0*AV_PI/180.0)    
    {    c_sail_lift     = 0.0;}
    else if (fabs(d2aoa) <= 25.0*AV_PI/180.0)
    {    c_sail_lift     = (2.24*fabs(d2aoa)-2.24*2.0*AV_PI/180.0);}  
    else if ((fabs(d2aoa) > 25.0*AV_PI/180.0) && (fabs(d2aoa) <= 90.0*AV_PI/180.0))
    {    c_sail_lift     = -0.79*fabs(d2aoa)+1.1;}           
    else if ((fabs(d2aoa) > 90.0*AV_PI/180.0) && (fabs(d2aoa) <= AV_PI))
    {    c_sail_lift     = 0.0;}
    
    c_sail_drag          = 1.28*sin(fabs(-d_wind_r+aoa));
    F_lift_V_w           = 1.0/2.0*dens_air*c_sail_lift*V_wind*V_wind*A_sail*cos(-d_wind_r+aoa);
    F_drag_V_w           = 1.0/2.0*dens_air*c_sail_drag*V_wind*V_wind*A_sail*sin(-d_wind_r+aoa)*signum(-d_wind_r+aoa);
// rtx_message("c_sail_lift %f, c_sail_drag %f, F_lift %f, F_drag %f",c_sail_lift, c_sail_drag, F_lift_V_w, F_drag_V_w);
    if (d_wind_r >= 0.0)
    {    vorzeichen = 1; }
    else
    {    vorzeichen = -1;    }

    if (fabs(aoa) <= 2.0*AV_PI/180.0)
    {     x_to_sail_coa = 0.0; y_to_sail_coa = 0; }
    else if (fabs(aoa) <= 40.0*AV_PI/180.0)                           
    {     x_to_sail_coa = 0.2; y_to_sail_coa = 0.3; }
    else if (fabs(aoa) > 40.0*AV_PI/180.0 && fabs(aoa) <= 70.0*AV_PI/180.0)
    {     x_to_sail_coa = 0.3; y_to_sail_coa = 0.35; }
    else if (fabs(aoa) > 70.0*AV_PI/180.0 && fabs(aoa) <= 120.0*AV_PI/180.0)
    {     x_to_sail_coa = 0.4; y_to_sail_coa = 0.4;  }
    else if (fabs(aoa) > 120.0*AV_PI/180.0 && fabs(aoa) <= 150.0*AV_PI/180.0)
    {     x_to_sail_coa = 0.2; y_to_sail_coa = 0.3;  }
    else if (fabs(aoa) > 150.0*AV_PI/180.0 && fabs(aoa) <= AV_PI)
    {     x_to_sail_coa = 0.0; y_to_sail_coa = 0.0;  }

    X_sail              = F_lift_V_w*sin(fabs(d_wind_r)) - F_drag_V_w*cos(d_wind_r);
    Y_sail              = (F_lift_V_w*cos(d_wind_r) + F_drag_V_w*sin(fabs(d_wind_r)))*signum(-vorzeichen);
    N_sail              = X_sail*x_to_sail_coa*signum(aoa) + Y_sail*y_to_sail_coa;
    sail_factor         = 0.1;
    N_sail              = sail_factor*N_sail;
// rtx_message("aoa: %f  d_wind_r: %f  sail: %f\n",aoa, d_wind_r, N_sail);
// N_sail=0;
    N_des               = 0.9*torque_des;//*0.81;
    N_rudder_des        = -N_sail + N_damping + N_des;
    Y_rudder            = -N_rudder_des/1.7;  //+??
//     Y_rudder_right      = Y_rudder/2.0;
// rtx_message("desired: %f  damping: %f  sail: %f  rudder: %f, rudderangle: %f",N_des, N_damping, N_sail, N_rudder_des, x*180/AV_PI);
    v_r_tot         = sqrt((speed_x*speed_x) + ((speed_y - 1.7*heading_speed)*(speed_y - 1.7*heading_speed)));
    d_water         = atan2((speed_y - 1.7*heading_speed),speed_x); 
//     d_water         = remainder(d_water,2*AV_PI);
//     k               = 0.5*dens_water*v_r_tot*v_r_tot*A_rudder;
    incid_angle     = remainder(-d_water + x,2*AV_PI);
//     incid_angle     = remainder(incid_angle,2*AV_PI);

// rtx_message("v_r_tot: %f  d_water: %f  incid: %f rudder: %f",v_r_tot, d_water, incid_angle, x);
    if (incid_angle >= 0.0)
    {     vorzeichenR  = 1; }
    else
    {     vorzeichenR  = -1;}
    if (d_water >= 0.0)
    {     vorzeichenRR = 1;} 
    else
    {     vorzeichenRR = -1;  }

// Error function of Fabian Jenne
//     double output = k*(1.9*(1.0-exp(-fabs(remainder((-d_water + x)*180.0/AV_PI,360.0)*AV_PI/180.0)*9.0))-2.4*fabs(remainder((-d_water + x)*180.0/AV_PI,360.0)*AV_PI/180.0))*cos(-d_water+x)*cos(d_water)*signum(-vorzeichenR) + k*1.28*sin(fabs(remainder((-d_water + x)*180.0/AV_PI,360.0)*AV_PI/180.0))*sin(-d_water+x)*signum(vorzeichenR)*sin(fabs(d_water))*signum(-vorzeichenRR) - Y_rudder_right;
//     double output = k*(1.9*(1.0-exp(-fabs(remainder((-d_water + x*180.0/AV_PI),360.0)*AV_PI/180.0)*9.0))-2.4*fabs(remainder((-d_water + x*180.0/AV_PI),360.0)*AV_PI/180.0))*cos(-d_water+x)*cos(d_water)*signum(-vorzeichenR) + k*1.28*sin(fabs(remainder((-d_water + x*180.0/AV_PI),360.0)*AV_PI/180.0))*sin(-d_water+x)*signum(vorzeichenR)*sin(fabs(d_water))*signum(-vorzeichenRR) - Y_rudder_right;
    //return 0.5*dens_water*v_r_tot*v_r_tot*A_rudder;//k*(1.9*(1-exp(-fabs(remainder((-d_water + x*180.0/AV_PI),360.0)*AV_PI/180.0)*9))-2.4*fabs(remainder((-d_water + x*180.0/AV_PI),360.0)*AV_PI/180.0));
	//


    // same model as in the matlab-file
    // c_rudder_drag = 1.28*sin(fabs(incid_angle)); //from Fabian
    // c_rudder_lift = 1.9*(1-exp(-fabs(incid_angle)*9))-2.4*fabs(incid_angle); //from Fabian
    c_rudder_drag = 0.1+0.3*pow(incid_angle,2);; //from Mario
    c_rudder_lift = fabs(7.1*fabs(incid_angle) - 4*pow(incid_angle,2) -16.6*pow(fabs(incid_angle),3)); //from Mario
// rtx_message("c_lift: %f", c_rudder_lift);
// rtx_message("c_r_drag: %f, c_r_lift: %f incid: %f",c_rudder_drag,c_rudder_lift, incid_angle);
    F_lift_v_right = 1.0/2.0*dens_water*c_rudder_lift*v_r_tot*v_r_tot*A_rudder*sin(incid_angle);
    F_drag_v_right = 1.0/2.0*dens_water*c_rudder_drag*v_r_tot*v_r_tot*A_rudder*sin(incid_angle)*vorzeichenR;

    F_lift_v_left = 1.0/2.0*dens_water*c_rudder_lift*v_r_tot*v_r_tot*A_rudder*sin(incid_angle);
    F_drag_v_left = 1.0/2.0*dens_water*c_rudder_drag*v_r_tot*v_r_tot*A_rudder*sin(incid_angle)*vorzeichenR;
// rtx_message("F_lift_R: %f, F_drag_R: %f, F_lift_L: %f, F_drag_L: %f\n",F_lift_v_right,F_drag_v_right,F_lift_v_left,F_drag_v_left);

    Y_rudder_right = F_lift_v_right*cos(d_water) + F_drag_v_right*sin(-d_water);
    double Y_rudder_left = F_lift_v_left*cos(d_water) + F_drag_v_left*sin(-d_water);
// rtx_message("rudderforceR: %f, rudderforceR: %f\n",Y_rudder_right,Y_rudder_left);
    double output = (Y_rudder_right+Y_rudder_left-Y_rudder)/Y_rudder;

#ifdef ROOT_FINDING
	// do nothing
#else
	// Square the value to help minimisation
	output *= output;
#endif
// rtx_message("v_r_tot: %f  d_water: %f  incid: %f rudder: %f force: %f error: %f",v_r_tot, d_water, incid_angle, x*180/AV_PI, Y_rudder_right*2, output);
// rtx_message("v_r_tot: %f  speed_x: %f  speed_y: %f heading_speed: %f rudder: %f error: %f",v_r_tot, speed_x, speed_y, heading_speed, x, output);
// rtx_message("des: %f  damp: %f  sail: %f  rud: %f, rud_t: %f ang: %f err: %f\n",N_des, N_damping, N_sail, N_rudder_des, -Y_rudder_right*3.4, x*180/AV_PI, output);
// rtx_message("rudderforce: %f, error: %f\n",Y_rudder,output);
// rtx_message("des_force: %f, real_force: %f, rudderangle: %f, error: %f",Y_rudder, Y_rudder_right, x*180/AV_PI, output);
// rtx_message("out: %f,   x: %f", output, x);
// rtx_message("rudder_T: %f, rudderangle: %f, error: %f",N_rudder_des, x*180/AV_PI, output);
	return output;
}

int signum(double i) 
{
    if (i>=0.0)
        return 1;
    else 
        return -1;
}




