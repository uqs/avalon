// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "avalon.h"
#include "sailor_rudder_iter_fn.h"
#include "sailor_main_iter_class.h" 


// General gsl-Things
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_min.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

// #define ROOT_FINDING
// #define MINIMIZING
//sailor_main_iter_class::sailor_main_iter_class()
//{
//}

double sailor_main_iter_class::sailor_main_iter_fn(double heading_speed, double torque_des, 
		double speed_x, double speed_y, double aoa_sail, 
		double d_wind, double pose_3, double V_wind )
{
	iter_start = atan2(((speed_y - 1.7*heading_speed)*0.01),speed_x);
	iter = 0, max_iter = 100;
	// const gsl_root_fsolver_type *T;
	// gsl_root_fsolver *s;
	// alpha_r = 0;

	x_lo = iter_start - 10.0*M_PI/180;//iter_start - 15*M_PI/180; 
	x_hi = iter_start + 10.0*M_PI/180;//iter_start + 15*M_PI/180;
	gsl_function F;

	struct rudder_iter_params params = {heading_speed, torque_des, speed_x, speed_y, aoa_sail, d_wind, pose_3, V_wind};

	F.function = &sailor_rudder_iter_fn;
	F.params   = &params;

#ifdef ROOT_FINDING
	const gsl_root_fsolver_type *T;
	gsl_root_fsolver *s;

	T = gsl_root_fsolver_brent;
	s = gsl_root_fsolver_alloc (T);
	gsl_root_fsolver_set (s, &F, x_lo, x_hi);

	do
	{
		iter++;
		status   = gsl_root_fsolver_iterate (s);
		x        = gsl_root_fsolver_root (s);
		x_lo     = gsl_root_fsolver_x_lower (s);
		x_hi     = gsl_root_fsolver_x_upper (s);
		status   = gsl_root_test_interval (x_lo, x_hi, 0, 0.1);
		alpha_r  = x;
	}

	while (status == GSL_CONTINUE && iter < max_iter);

	gsl_root_fsolver_free (s);
#else 
#ifdef MINIMIZING// Minimisation
	const gsl_min_fminimizer_type *T;
	gsl_min_fminimizer *s;
// rtx_message("-3");
	T = gsl_min_fminimizer_brent;
	s = gsl_min_fminimizer_alloc (T);
// rtx_message("-2");
//check if there is a minimum
double x_med=(x_lo+x_hi)/2;
double output_lo = sailor_rudder_iter_fn(x_lo, &params);
double output_hi = sailor_rudder_iter_fn(x_hi, &params);
double output_med = sailor_rudder_iter_fn(x_med, &params);
double output_med2;
if (output_lo == output_hi && output_hi == output_med)
{
    return x_lo*180/M_PI;
}

if (output_lo < output_hi)
{
// rtx_message("check if there is a minimum: x_min (%f)-> %f   x_med (%f)-> %f   x_max (%f)-> %f", x_lo, output_lo,(x_lo+x_hi)/2, output_med, x_hi, output_hi);
    if (output_lo < output_med)
    {
	output_med2 = sailor_rudder_iter_fn((x_lo+x_med)/2, &params);
	if (output_lo < output_med2)
	{
	  return x_lo*180/M_PI;
	}
	else
	{
	    x_hi=x_med;
	    output_hi=output_med;
	}
    }
}
else
{
// rtx_message("check if there is a minimum: x_min (%f)-> %f   x_med (%f)-> %f   x_max (%f)-> %f", x_lo, output_lo,(x_lo+x_hi)/2, output_med, x_hi, output_hi);
    if (output_hi < output_med)
    {
	output_med2 = sailor_rudder_iter_fn((x_hi+x_med)/2, &params);
	if (output_hi < output_med2)
	{
	  return x_hi*180/M_PI;
	}
	else
	{
	    x_lo=x_med;
	    output_lo=output_med;
	}
    }
}
// rtx_message("check if there is a minimum: x_min (%f)-> %f   x_max (%f)-> %f", x_lo*180/AV_PI, output_lo, x_hi*180/AV_PI, output_hi);
	
// rtx_message("-1");
gsl_min_fminimizer_set (s, &F, iter_start, x_lo, x_hi);

	do
	{
// rtx_message("0");
		iter++;
		status = gsl_min_fminimizer_iterate (s);
		x = gsl_min_fminimizer_x_minimum (s);
		x_lo = gsl_min_fminimizer_x_lower (s);
		x_hi = gsl_min_fminimizer_x_upper (s);
// rtx_message("1");
		status = gsl_min_test_interval (x_lo, x_hi, 0.001, 0.0);
		alpha_r  = x;
// rtx_message("2");
	}
	while (status == GSL_CONTINUE && iter < max_iter);
// rtx_message("3");
	gsl_min_fminimizer_free (s);
// rtx_message("4");
#else
double iteration_gap = 0.1*AV_PI/180;
int num_iter = int((x_hi-x_lo)/iteration_gap);
int i = 1;

double x_iter = x_lo;
// double alpha_r;
double err_min = sailor_rudder_iter_fn(x_lo, &params);
double err;

while (i<num_iter)
{
	x_iter += iteration_gap;
	err = sailor_rudder_iter_fn(x_iter, &params);
	
	if (err < err_min)
	{
		err_min = err;
		alpha_r = x_iter;
	}
	i++;
}
// rtx_message("error: %f, alpha: %f",err_min, alpha_r*180/AV_PI);
#endif
#endif

	// return status;
	return alpha_r*180/AV_PI;
}

