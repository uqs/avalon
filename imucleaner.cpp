// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>
#include <rtx/filter.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "imucleaner.h"
#include "imu.h"

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataImu;
DDXVariable dataImuClean;

/**
 * Prototypes for utility functions
 * */
int sign(int i);
int sign(float i);
int sign(double i);

/**
 * Storage for the command line arguments
 * */
const char * varname_imu = "imu";
const char * varname_imuClean = "cleanimu";
const char * producerHelpStr = "Cleaning up the wind data from the sensor";

/**
 * Command line arguments
 *
 * */
RtxGetopt producerOpts[] = {
    {"imuname", "Store-Variable where the dirty imu data comes from",
        {
            {RTX_GETOPT_STR, &varname_imu, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"cleanimuname", "Store Variable where the cleaned imu data is written to",
        {
            {RTX_GETOPT_STR, &varname_imuClean, ""},
            RTX_GETOPT_END_ARG
        }
    },
    RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{
	imuData imu;
	imuCleanData imu_clean;
    imuCleanData imu_last;

    RtxFilter* velocity_x_filter = NULL;
    RtxFilter* velocity_y_filter = NULL;
    RtxFilter* velocity_z_filter = NULL;
    RtxFilter* roll_filter = NULL;
    RtxFilter* pitch_filter = NULL;
    RtxFilter* yaw_filter = NULL;
    bool firsttime = true;
    
    imu_clean.attitude.roll = 0.0;
    imu_clean.attitude.pitch = 0.0;
    imu_clean.attitude.yaw = 0.0;
    imu_clean.velocity.x = 0.0;
    imu_clean.velocity.y = 0.0;
    imu_clean.velocity.z = 0.0;
    imu_clean.velocity.drift = 0.0;

    imu_last.attitude.roll = 0.0;
    imu_last.attitude.pitch = 0.0;
    imu_last.attitude.yaw = 0.0;
    imu_last.velocity.x = 0.0;
    imu_last.velocity.y = 0.0;
    imu_last.velocity.z = 0.0;
    imu_last.velocity.drift = 0.0;

	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (dataImu.t_readto(imu,5.0,1))
		{
            if(firsttime)
            {
                // my_filter = rtx_filter_init_from_file("imucleanerparams.txt");
                // my_filter = rtx_filter_smooth1(0.5);
                velocity_x_filter = rtx_filter_median_init(500);
                velocity_y_filter = rtx_filter_median_init(100);
                velocity_z_filter = rtx_filter_median_init(100);
                roll_filter = rtx_filter_median_init(100);
                pitch_filter = rtx_filter_median_init(100);
                yaw_filter = rtx_filter_median_init(500);
                firsttime = false;
            }

            /**doing the right thing with the new values to avoid leaps (-180 to 180) - for every value, there is two operations:**/
            // roll:
            if(fabs(remainder(imu_last.attitude.roll,360.0) - imu.attitude.roll) > 180.0)
            {
                imu.attitude.roll += sign(remainder(imu_last.attitude.roll,360.0)) * 360.0;
            }
            imu.attitude.roll += (imu_last.attitude.roll - remainder(imu_last.attitude.roll,360.0));
            // pitch:
            if(fabs(remainder(imu_last.attitude.pitch,360.0) - imu.attitude.pitch) > 180.0)
            {
                imu.attitude.pitch += sign(remainder(imu_last.attitude.pitch,360.0)) * 360.0;
            }
            imu.attitude.pitch += (imu_last.attitude.pitch - remainder(imu_last.attitude.pitch,360.0));
            // yaw:
            if(fabs(remainder(imu_last.attitude.yaw,360.0) - imu.attitude.yaw) > 180.0)
            {
                imu.attitude.yaw += sign(remainder(imu_last.attitude.yaw,360.0)) * 360.0;
            }
            imu.attitude.yaw += (imu_last.attitude.yaw - remainder(imu_last.attitude.yaw,360.0));

            // save the new data in imu_last for the next iteration:
            imu_last.attitude.roll = imu.attitude.roll;
            imu_last.attitude.pitch = imu.attitude.pitch;
            imu_last.attitude.yaw = imu.attitude.yaw;

            // Evaluate Filter Function
            imu_clean.velocity.x = rtx_filter_median_step(velocity_x_filter, imu.velocity.x);
            imu_clean.velocity.y = rtx_filter_median_step(velocity_y_filter, imu.velocity.y);
            imu_clean.velocity.z = rtx_filter_median_step(velocity_z_filter, imu.velocity.z);
            imu_clean.attitude.roll = remainder(rtx_filter_median_step(roll_filter, imu.attitude.roll),360.0);
            imu_clean.attitude.pitch = remainder(rtx_filter_median_step(pitch_filter, imu.attitude.pitch),360.0);
            imu_clean.attitude.yaw = remainder(rtx_filter_median_step(yaw_filter, imu.attitude.yaw),360.0);

            // calculate additional values
            // drift probably not needed anymore, because IMU gives values in
            // north-south plane already, so velocity.y is all we need
            imu_clean.velocity.drift = sqrt(imu_clean.velocity.y * imu_clean.velocity.y + imu_clean.velocity.z * imu_clean.velocity.z) * sign(-imu_clean.attitude.roll);

			// Bring to store
			dataImuClean.t_writefrom(imu_clean);
		}
		else if (dataImu.hasTimedOut()) {
			rtx_message("Timeout while reading IMU-Data \n");}
		else {
			// Something strange happend. Critical Error.
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}
	}
	return NULL;
}

// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}}

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;}

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;}


// Some self-defined utility functions:
int sign(int i) // gives back the sign of an int
{
    if (i>=0)
        return 1;
    else 
        return -1;
}

int sign(float i) // gives back the sign of a float
{
    if (i>=0)
        return 1;
    else 
        return -1;
}

int sign(double i) // gives back the sign of a double
{
    if (i>=0)
        return 1;
    else 
        return -1;
}


int main (int argc, const char * argv[])
{
	RtxThread * th;
    int ret;

	// Process the command line
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("IMU Cleaner interface Main", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuData));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), imuCleanData));


	// Connect to variables, and create variables for the target-data
	DOB(store.registerVariable(dataImu, varname_imu, "imuData"));
	DOB(store.registerVariable(dataImuClean, varname_imuClean, "imuCleanData"));

	// Start the working thread
    DOP(th = rtx_thread_create ("Wind cleaning thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down imucleaner...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
