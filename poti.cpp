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






int main (int argc, const char * argv[])
{
    double angle;
    int num_tick;
    can_init("/dev/ttyUSB4");
    
	can_send_message(&msg_poti_pos_request);
    while (1)
    {
	num_tick = int(message.content[4])+int(message.content[5])*256+int(message.content[6])*256*256 + int(message.content[7])*256*256*256;
	angle = remainder((num_tick%AV_POTI_RESOLUTION)*360.0/AV_POTI_RESOLUTION,360.0);
    }
	// The destructors will take care of cleaning up the memory
    return (0);

}
