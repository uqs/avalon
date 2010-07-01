/**
 *	This reads the target angles form the Store and sets it on the Rudder-EPOS
 *
 **/

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Specific Things
// #include "poti.h"
#include "avalon.h"
#include "can/can.h"
#include "epos/epos.h"

static can_message_t msg = {
  0x0,
  {0x0, 0x0, 0x0, 0, 0, 0, 0, 0}
};

int poti_pos_request(double* angle)
{
    int num_tick;

    msg.id = 0x600 + AV_POTI_NODE_ID;
    msg.content[0] = 0x40;
    msg.content[1] = 0x04;
    msg.content[2] = 0x60;
    
    int ret=can_send_message(&msg);
    if (!ret)
    {
	num_tick = int(message.content[4])+int(message.content[5])*256+int(message.content[6])*256*256;
	*angle = remainder((num_tick%AV_POTI_RESOLUTION)*360.0/AV_POTI_RESOLUTION,360.0);
    }

    return ret;
}


int poti_set_PDO()
{
    msg.id = 0x600 + AV_POTI_NODE_ID;
    msg.content[0] = 0x01;
    msg.content[1] = 0x08;

    int ret=can_send_message(&msg);

    return ret;
}


int poti_init()
{
    can_init("/dev/ttyUSB0");
return 0;
}


int
main (int argc, char * argv[])
{
  int num_tick;
  double angle;
  if(argc < 2) {
    fprintf(stderr, "usage: %s DEV\n", argv[0]); 
    return -1;
  }
// poti_init();
  can_init(argv[1]);
  poti_pos_request(&angle);
//   poti_set_PDO();

  

while(1) {
  
    can_read_message();
  num_tick = int(message.content[4]) + int(message.content[5])*256 + int(message.content[6])*256*256 + int(message.content[7])*256*256*256;
  angle = remainder((num_tick%AV_POTI_RESOLUTION)*360.0/AV_POTI_RESOLUTION,360.0);
  if (message.id != 0)
    printf("Received 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X   ticks: %d  angle: %f°\n", message.id, message.content[0], message.content[1], message.content[2], message.content[3], message.content[4], message.content[5], message.content[6], message.content[7], num_tick, angle);
}

  can_close();
  return (0);
}

