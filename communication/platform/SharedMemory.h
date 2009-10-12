//
// C++ Interface: SharedMemory
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include "Exception.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <string.h>

#include <sstream>
//#include <iostream>

#define SHARED_MEMORY_TIMEOUT 10

/**
 * Encapsulates a variable shared by different processes. The memory segment is
 * identified by an id on which all processes must agree.
 *
 * Read and write access is protected by a single semaphore to avoid data
 * corruption.
 *
 * Beware that this is only suitable for data types which don't allocate memory
 * themselves. Such memory would not be shared in between processes and render
 * the object unusable.
 */
template <typename T> class SharedMemory
{
// data members
private:
	int key;
	int shmid;
	std::string semaphore_name;
	void *content;
	sem_t *semaphore;
// data types
public:
	enum ids {
		ID_POWER_DATA = 1,
		ID_GPS_DATA = 2,
		ID_ELECTRICAL_DATA = 3
	};

// methods
	/**
	 * Connect to the shared memory segment associated with the given key.
	 * @todo make this race-condition free
	 *
	 * @param id	ID of the memory segment
	 * @param initialize	If set, the memory segment is created and
	 *			initialized to 0. Otherwise it is assumed that
	 *			the segment already exists and the call fails
	 *			if it doesn't.
	 */
	SharedMemory(key_t id, bool initialize) : key(id)
	{
		// make a shared memory segment
		int flag = 0600;
		if(initialize) {
			flag |= IPC_CREAT;
		}
		shmid = shmget(id, sizeof(T), flag);
		if(shmid == -1) {
			std::ostringstream os;
			os << "cannot get shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
		
		content = shmat(shmid, 0, 0);
		if(content == (void *)-1) {
			std::ostringstream os;
			os << "cannot attach to shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
		
		// now a semaphore
		std::ostringstream o;
		o << "SharedMemory" << id << "lock";
		semaphore_name = o.str();
		semaphore = sem_open(semaphore_name.c_str(), O_CREAT, 0600 , 0);
		if(semaphore == SEM_FAILED) {
			std::ostringstream os;
			os << "creating the semaphore for shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
		
		// now initialize if needed
		if(initialize) {
			memset(content, 0, sizeof(T));
			while(sem_trywait(semaphore) == 0);
			if(sem_post(semaphore) == -1) {
				std::ostringstream os;
				os << "resetting the semaphore for shared memory segment " << key;
				throw(Exception(os.str(), errno));
			}
		}
	}

	~SharedMemory()
	{
		shmdt(content);
		sem_unlink(semaphore_name.c_str());
	}

	/**
	 * Read and return the current value.
	 */
	T read()
	{
//		int value;
		struct timespec timeout;
		T ret;
		timeout.tv_sec = time(NULL) + SHARED_MEMORY_TIMEOUT;
		timeout.tv_nsec = 0;
//		sem_getvalue(semaphore, &value);
//		std::cout << "before read: " << value << std::endl;
		if(sem_timedwait(semaphore, &timeout) != 0) {
			std::ostringstream os;
			os << "locking semaphore for reading shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
		memcpy(&ret, content, sizeof(T));
		if(sem_post(semaphore) == -1) {
			std::ostringstream os;
			os << "unlocking semaphore for reading shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
//		sem_getvalue(semaphore, &value);
//		std::cout << "after read: " << value << std::endl;
		
		return ret;
	}

	/**
	 * Set the new value
	 *
	 * @param v
	 */
	void write(const T &v)
	{
//		int value;
		struct timespec timeout;
		timeout.tv_sec = time(NULL) + SHARED_MEMORY_TIMEOUT;
		timeout.tv_nsec = 0;
//		sem_getvalue(semaphore, &value);
//		std::cout << "before write: " << value << std::endl;
		if(sem_timedwait(semaphore, &timeout) != 0) {
			std::ostringstream os;
			os << "locking semaphore for writing shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
		memcpy(content, &v, sizeof(T));
		if(sem_post(semaphore) == -1) {
			std::ostringstream os;
			os << "unlocking semaphore for writing shared memory segment " << key;
			throw(Exception(os.str(), errno));
		}
//		sem_getvalue(semaphore, &value);
//		std::cout << "after write: " << value << std::endl;
	}
};

#endif
