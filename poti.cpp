/**
 * Skipper calls the navigations-programs and sets a current heading to the
 * store, so sailor can take over!!
 *
 **/
// TODO check "remainder", speed history, which obstacles do we care about, 

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>


// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/time.h>
#include <rtx/thread.h>
#include <rtx/message.h>

// Specific Things

#include <DDXStore.h>
#include <DDXVariable.h>

#include "flags.h"
#include "poti.h"
#include "ports.h"
#include "can/can.h"
#include "epos/epos.h"

// #define DEBUG_AISEVAL

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable dataFlags;
DDXVariable potiData;
DDXVariable dataPorts;


/**
 * Storage for the command line arguments
 * */

const char * varname_potiData = "potiData";
const char * varname_flags = "flags";
const char * varname_ports = "ports";
const char * producerHelpStr = "poti help-string";

static can_message_t msg_clear = {
  0x0,
  {0x0, 0x0, 0x0, 0, 0, 0, 0, 0}
};
static can_message_t msg_poti_pos_request = {
  0x600 + AV_POTI_NODE_ID,
  {0x40, 0x04, 0x60, 0, 0, 0, 0, 0}
};
static can_message_t msg_poti_set_pdo = {
  0x600 + AV_POTI_NODE_ID,
  {0x01, 0x08, 0x0, 0, 0, 0, 0, 0}
};

RtxGetopt producerOpts[] = {

//   {"imuData", "Store Variable where the imuData is written",
//    {
//      {RTX_GETOPT_STR, &varname4, "imuData"},
//      RTX_GETOPT_END_ARG
//    }
//   },
// 
//   {"cleanimuname", "Store Variable where the cleaned imu data is written to",
//    {
//      {RTX_GETOPT_STR, &varname_imuClean, ""},
//      RTX_GETOPT_END_ARG
//    }
//   },
  RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{

    Flags generalflags;
    PotiData poti;
    Ports ports;

    char commport[99] = "auto";
    int num_tick;
    double angle;

    
    dataPorts.t_readto(ports,0,0);
    sprintf(commport, "/dev/ttyUSB%d", ports.sail);
    
    can_init("commport");
    
    while (1)
    {
	can_send_message(&msg_poti_pos_request);
	num_tick = int(message.content[4])+int(message.content[5])*256+int(message.content[6])*256*256 + int(message.content[7])*256*256*256;
	angle = remainder((num_tick%AV_POTI_RESOLUTION)*360.0/AV_POTI_RESOLUTION,360.0);
	poti.sail_angle_abs = angle;
	potiData.t_writefrom(poti);
    }
  
}

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
    if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("AIS Eval Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

// Register the new Datatypes
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), Flags));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), PotiData));

    //
	
    // Create output variable
	DOB(store.registerVariable(dataFlags, varname_flags, "Flags"));
        DOB(store.registerVariable(potiData, varname_potiData, "PotiData"));

	// Start the working thread
    DOP(th = rtx_thread_create ("aisEval thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down aisEval...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);

}
