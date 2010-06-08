/**
 * Skipper calls the navigations-programs and sets a current heading to the
 * store, so sailor can take over!!
 *
 **/

// General Project Constants
#include "avalon.h"

// General Things
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>

// General rtx-Things
#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>

#include <DDXStore.h>
#include <DDXVariable.h>

#include "destination.h"


// #define DEBUG_DEST_CONVERT

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable destinationData;
DDXVariable destinationStruct; //the curr heading will be written here; sailor needs that!!!


/**
 * Prototypes for utility functions
 * */
//int sign(int i);
//int sign(float i);


/**
 * Storage for the command line arguments
 * */


const char * varname_destData = "destData";
const char * varname_destStruct = "destStruct";


const char * producerHelpStr = "skipper help-string";

/**
 * Command line arguments   //has yet to be completed
 *
 * */
RtxGetopt producerOpts[] = {
#if 0
  {"destinationData","Variable where the destination-Data is stored",
   {
     {RTX_GETOPT_STR, &varname_destData, "DestinationData"},
     RTX_GETOPT_END_ARG
   }
  },
#endif
   RTX_GETOPT_END
};


/**
 * Working thread, wait the data, transform them and write them again
 * */
void * translation_thread(void * dummy)
{
    //Destination destinationCoord;
    DestinationData destination;

    int current_destpoint = 0;

    
	while (1) {
		// Read the next data available, or wait at most 5 seconds
		if (1)
		{
            
            std::ifstream destfile ("destination.txt");
            int navigation_type;    // if 1: normal destination-approach, if 2 -> buoy approach!
            float longitude,latitude;

            char currentLine[10000], *ptr;
            if (destfile.is_open())
            {
                    
                //start everything:

                destfile.getline (currentLine,30,'\n');
                
                navigation_type = strtol(currentLine,&ptr,0);
                
                destfile.getline (currentLine,30,'\n');
                destfile.getline (currentLine,30,'\n');
                
                //normal destination-approach:
                if(navigation_type ==1)
                {
                    longitude = strtod(currentLine,&ptr);
                    latitude = strtod(ptr,NULL);

                    // write to store:

#ifdef DEBUG_DEST_CONVERT
                    rtx_message("destination longitude = %f, latitude = %f, \n",longitude,latitude);
#endif
//                     destination.Data[0].longitude = double (AV_EARTHRADIUS 
//                         *cos((latitude * AV_PI/180))*(AV_PI/180)
//                         *longitude);
//                     destination.Data[0].latitude =double (AV_EARTHRADIUS
//                         *(AV_PI/180)*latitude);
		    destination.Data[0].longitude = double (longitude);
                    destination.Data[0].latitude =double (latitude);

                    destination.Data[0].passed = 0;
                    destination.Data[0].type = AV_DEST_TYPE_END;
                    destination.Data[1].type = AV_DEST_TYPE_NOMORE;

		    // set the first destination point to the current one
		    destination.longitude = destination.Data[0].longitude;
		    destination.latitude = destination.Data[0].latitude;
                    //bring to store:

                    destinationData.t_writefrom(destination);

                    //abort programm:
                    destfile.close();
                    assert(0);

                }
                
                if(navigation_type == 2)
                {
                    while(currentLine[0]!='#' && current_destpoint < 1000)
                    {
                        longitude = strtod(currentLine,&ptr);
                        latitude = strtod(ptr,NULL);

                        // write to store:

#ifdef DEBUG_DEST_CONVERT
                        rtx_message("destination longitude = %f, latitude = %f, curent_destpoint = %d \n",longitude,latitude, current_destpoint);
#endif

//                         destination.Data[current_destpoint].longitude = (AV_EARTHRADIUS 
//                             *cos((latitude * AV_PI/180))*(AV_PI/180) *longitude);
//                         destination.Data[current_destpoint].latitude = (AV_EARTHRADIUS
//                             *(AV_PI/180)*latitude);

			destination.Data[current_destpoint].longitude = double (longitude);
                        destination.Data[current_destpoint].latitude = double (latitude);

                        destination.Data[current_destpoint].passed = 0;
                        destination.Data[current_destpoint].type = AV_DEST_TYPE_OCEANWYP;

                        //bring to store:

                        destinationData.t_writefrom(destination);

                        destfile.getline (currentLine,30,'\n');
                        current_destpoint ++;

                    }

     		    // set the second destination point to the current one
		    destination.longitude = destination.Data[1].longitude;
		    destination.latitude = destination.Data[1].latitude;

                    destination.Data[current_destpoint-1].type = AV_DEST_TYPE_END;
                    destination.Data[current_destpoint].type = AV_DEST_TYPE_NOMORE;

                    //bring to store:

                    destinationData.t_writefrom(destination);
             rtx_message("it should end here, %d destinations written", current_destpoint); 
                    //abort programm:
                    destfile.close();
                    rtx_main_signal_shutdown();

                }
            }
            else rtx_message("unable to open destination-file");
        }

        //TODO: has to be modified:


        else
		{
			// Something strange happend. Critical Error.
			rtx_error("Critical error while reading data");
			// Emergency-Stop
			rtx_main_signal_shutdown();
		}
            rtx_timer_sleep(1);
	}

	return NULL;
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
	rtx_main_init ("Simulator Interface", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the new Datatypes
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationStruct));
    DOC(DDX_STORE_REGISTER_TYPE (store.getId(), DestinationData));



	// Connect to variables, and create variables for the target-data
    //destination of AVALON:
	DOB(store.registerVariable(destinationData, varname_destData, "DestinationData"));
	DOB(store.registerVariable(destinationStruct, varname_destStruct, "DestinationStruct"));


	// Start the working thread
    DOP(th = rtx_thread_create ("dest_converter thread", 0,
								RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0,
								RTX_THREAD_CANCEL_DEFERRED,
								translation_thread, NULL,
								NULL, NULL));

	// Wait for Ctrl-C
    DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Ctrl-C detected. Shutting down destination_converter...");

	// Terminating the thread
    rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
    return (0);
}
