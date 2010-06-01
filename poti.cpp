/**
 *	This reads the target angles form the Store and sets it on the Rudder-EPOS
 *
 **/

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Specific Things
#include "can/can.h"
#include "can/epos.h"

int
main (int argc, char * argv[])
{
  can_init("/dev/ttyUSB0");

  while(1) {
    can_read_message();
//     if (epos_read.node[0].msg_id != 0)
//       printf("Received 0x%X", epos_read.node[0].msg_id);
  }

  can_close();
  return (0);
}

