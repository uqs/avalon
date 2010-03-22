// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// General gsl-Things
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

// Specific Things
#include <rtx/pid.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "flags.h"
#include "sail-target.h"
#include "rudder-target.h"
#include "Ship.h"
#include "Sailstate.h"
#include "Rudderstate.h"
#include "windcleaner.h"
#include "imu.h"
#include "imucleaner.h"
#include "desired_course.h"

#include "sailor_rudder_iter_fn.h"
#include "sailor_main_iter_class.h"


  int main()
  {
     sailor_main_iter_class iter;

   // variables   
  double u;
   // input
  double head_speed = 0;
  double des_torque = 30.0;
  double x_speed    = 4.6;
  double y_speed    = 0.0;
  double sail_ang   = 10.0;
  double wind_ang   = 45.0;
  double head_pose  = 7.0;
  double wind_speed = 13.0;
  
  rudder_iter_params params = {head_speed, des_torque, x_speed, y_speed, sail_ang, wind_ang, head_pose, wind_speed};
  
  FILE * fp = fopen("fn","w");
  for (double x=-M_PI/3.0;x<M_PI/3.0;x+=0.003) { 
	fprintf(fp,"%f %f\n",x,sailor_rudder_iter_fn(x, &params));
}
fclose(fp);
   // computation
   u = iter.sailor_main_iter_fn(head_speed*M_PI/180.0, des_torque, x_speed*0.5144, y_speed*0.5144, sail_ang*M_PI/180.0, wind_ang*M_PI/180.0, head_pose*M_PI/180, wind_speed*0.5144);

   rtx_message("u_zero = %f \n",u);

  };

