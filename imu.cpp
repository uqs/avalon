/************************************************************************/
/*									                                    */
/*		               P R O J E K T    A V A L O N 			        */
/*								 	                                    */
/*	    imu.cpp		Producer for the IMU. Captures all Data and         */
/*			        brings it to the Store			                    */
/*									                                    */
/*      Authors     Patrick Schwizer        patricsc@student.ethz.ch    */
/*                  Stefan Wismer           wismerst@student.ethz.ch    */
/*									                                    */
/************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <curses.h>
#include <math.h>

//general includes:
#include "avalon.h"

#include <DDXStore.h>
#include <DDXVariable.h>

#include <string.h>
#include <iostream>


#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include "cmt1.h"
#include "cmt2.h"
#include "cmt3.h"
#include "cmtdef.h"
#include "cmtf.h"
#include "cmtmessage.h"
#include "cmtpacket.h"
#include "cmtscan.h"
#include "pstdint.h"
#include "xsens_fifoqueue.h"
#include "xsens_janitors.h"
#include "xsens_list.h"
#include "xsens_list.hpp"
#include "xsens_std.h"
#include "xsens_time.h"

#include "imu.h"
using namespace xsens;
using namespace std;
/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataV;

Cmt3 serial;

// Storage for the command line arguments
const char * varname = "imu";
const char * portname = "/dev/xsensIMU"; // HE: made a udev rule for the IMU
const char * producerHelpStr = "All Data coming from the IMU";
double frequency = 100;

// Command line arguments processing
RtxGetopt producerOpts[] = {
  {"name", "name of the variable to register",
   {
     {RTX_GETOPT_STR, &varname, ""},
     RTX_GETOPT_END_ARG
   }
  },
  {"port", "name of the comm port to use",
   {
     {RTX_GETOPT_STR, &portname, ""},
     RTX_GETOPT_END_ARG
   }
  },
 {"frequency", "frequency of new data",
   {
     {RTX_GETOPT_STR, &frequency, ""},
     RTX_GETOPT_END_ARG
   }
  },
  RTX_GETOPT_END
};

#define EXIT_ERROR(loc) {printf("Error %d occurred during " loc ": %s\n", serial.getLastResult(), xsensResultText(serial.getLastResult())); exit(-1); }

int quit = 0;

void exitFunc(void)
{
    (void) signal(SIGINT, SIG_DFL);
}


void * imu_production_thread(void * dummy);


// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}


int main (int argc, const char * argv[])
{

	RtxThread * th;
	int ret;

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL,
					producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}

	if (serial.openPort(portname, B115200) != XRV_OK){
		EXIT_ERROR("open");}

	printf("MT now in config mode\n");

	CmtDeviceMode mode(CMT_OUTPUTMODE_ORIENT | CMT_OUTPUTMODE_TEMP | CMT_OUTPUTMODE_POSITION | CMT_OUTPUTMODE_VELOCITY | CMT_OUTPUTMODE_CALIB,
			CMT_OUTPUTSETTINGS_TIMESTAMP_SAMPLECNT | CMT_OUTPUTSETTINGS_ORIENTMODE_EULER,
			frequency);

	if (serial.setDeviceMode(mode, false, CMT_DID_BROADCAST)){
		EXIT_ERROR("set device mode");}
	
	printf("Device modes set\n");

	rtx_main_init ("IMU-producer", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the DataExample data type
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));

	// Create output variable
	DOB(store.registerVariable(dataV,varname,"imuData"));

	// Start the working thread
	DOP(th = rtx_thread_create ("thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
				RTX_THREAD_CANCEL_DEFERRED,
				imu_production_thread, NULL,
				NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Caught SIGINT/SIGQUIT, exiting ...");

	//close the Port
	serial.closePort();

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
	return (0);
}

void * imu_production_thread(void * dummy)
{
	imuData imu;				//imu-struct
    double velocity_to_north, velocity_to_west;

	Packet reply(1,0); /* 1 item, not xbus */
FILE * imu_data;
	if (serial.gotoMeasurement()){
		EXIT_ERROR("goto measurement");}

	printf("Now in measurement mode\n");

	//long msgCount = 0;


	while (1)
	{
		/*if (serial.waitForDataMessage(&reply) != XRV_OK)
		  {
		  printf("\n");
		  EXIT_ERROR("read data message");
		  }*/
		serial.waitForDataMessage(&reply);
		/* TODO CP: handle errors here. If there is errors, it is very 
		 * dangerous to write some false values in the imu variable */
imu_data=fopen("imu_plot.txt","a+");
		imu.attitude.roll=reply.getOriEuler().m_roll;   		// roll
		imu.attitude.pitch=reply.getOriEuler().m_pitch;    		// pitch
		imu.attitude.yaw=-remainder(reply.getOriEuler().m_yaw,360.0);    // yaw, Avalon-convention

fprintf(imu_data,"%f %f %f %f %f \n",reply.getVelocity().m_data[0], reply.getVelocity().m_data[1], imu.attitude.yaw/100.0, imu.position.latitude, imu.position.longitude);
fclose(imu_data);
		imu.position.latitude=reply.getPositionLLA().m_data[0];
		imu.position.longitude=reply.getPositionLLA().m_data[1];
		imu.position.altitude=reply.getPositionLLA().m_data[2];

        // velocities read from the IMU are in world coordinates. --> x =
        // north, y = west. The following conversion is assuming that the boat
        // never rolls - it neglects velocity in z (boat) direction
		velocity_to_north = 3.6*0.539956803*reply.getVelocity().m_data[0]; 	// in knots
		velocity_to_west = 3.6*0.539956803*reply.getVelocity().m_data[1]; 	// in knots
		imu.velocity.x = velocity_to_north * cos(imu.attitude.yaw * AV_PI / 180.0) - velocity_to_west * sin(imu.attitude.yaw * AV_PI / 180.0); 	// in knots
		imu.velocity.y = velocity_to_north * sin(imu.attitude.yaw * AV_PI / 180.0) + velocity_to_west * cos(imu.attitude.yaw * AV_PI / 180.0); 	// in knots
		imu.velocity.z = 3.6*0.539956803*reply.getVelocity().m_data[2];	// in knots
	
		imu.speed=sqrt(imu.velocity.x*imu.velocity.x + imu.velocity.y*imu.velocity.y);

		imu.gyro.x=reply.getCalGyr().m_data[0];
		imu.gyro.y=reply.getCalGyr().m_data[1];
		imu.gyro.z=-180.0/AV_PI*reply.getCalGyr().m_data[2];

		imu.acceleration.x=-reply.getCalAcc().m_data[0];
		imu.acceleration.y=-reply.getCalAcc().m_data[1];
		imu.acceleration.z=-reply.getCalAcc().m_data[2];

		imu.temperature=reply.getTemp();

		dataV.t_writefrom(imu);

		//rtx_message("roll: %6.1f, longitude: %6.1f",imu.attitude.roll,imu.position.longitude);



	}
	return 0;
}

