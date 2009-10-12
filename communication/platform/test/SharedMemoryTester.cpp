//
// C++ Implementation: SharedMemoryTester
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SharedMemoryTester.h"
#include "SharedMemory.h"

#include <iostream>

using namespace std;

SharedMemoryTester::SharedMemoryTester()
 : Tester()
{
}


SharedMemoryTester::~SharedMemoryTester()
{
}


bool SharedMemoryTester::test()
{
	string s;
	bool server;
	int id;
	struct shmid_ds ds;
	cout << "enter string id: " << flush;
	cin >> id;
	
	cout << "master? " << flush;
	cin >> server;
	
	SharedMemory<struct shmid_ds> shm(id, server);
	
	if(server) {
		ds.shm_dtime = 333;
		
		shm.write(ds);
		
		cout << "waiting for slave to run" << endl;
		while(shm.read().shm_dtime != time(NULL)) {
			sleep(1);
		}
	}
	else {
		ds = shm.read();
		cout << "value read: " << ds.shm_dtime << endl;
		ds.shm_dtime = time(NULL) + 10;
		shm.write(ds);
	}
	
	return true;
}

