/**
 *	This reads the target angles form the Store and sets it on the Rudder-EPOS
 *
 **/

// General Things
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// Specific Things
#include "Motor.h"

#define ID 0x0001


int
main (int argc, char * argv[])
{
	Motor m;

	m.init((argc<=1)?"/dev/ttyUSB0":argv[1], (argc<=2)?ID:atoi(argv[2]));

	printf("Position Mode\n");
	m.setPositionMode();
	for (unsigned int i=0;i<3;i++) {
		printf("Position %d\n",i);
		m.moveToAngle(i*2000);
		for (unsigned int j=0;j<20;j++) {
			printf("Position %.2f Speed %.2f\n",m.getPosition(),m.getVelocity());
			usleep(50000);
		}
	}

	printf("Velocity Mode\n");
	m.setVelocityMode();
	m.moveToSpeed(300);
	for (unsigned int j=0;j<60;j++) {
		printf("Position %.2f Speed %.2f\n",m.getPosition(),m.getVelocity());
		usleep(50000);
	}

	printf("Position Mode\n");
	m.setPositionMode();
	for (unsigned int i=0;i<3;i++) {
		printf("Position %d\n",i);
		m.moveToAngle((2-i)*2000);
		for (unsigned int j=0;j<20;j++) {
			printf("Position %.2f Speed %.2f\n",m.getPosition(),m.getVelocity());
			usleep(50000);
		}
	}


	m.emergencyStop();

    return (0);
}
