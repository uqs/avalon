#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>


/**
 * C++ interface to use POSIX threads.
 *
 * This is an abstract class. Objects wishing to implement tread behaviour
 * should inherit from this class and overload the run() method.
 */
class Thread
{
protected:
	pthread_t thread_id;
	volatile bool done;

public:
	Thread();
	virtual ~Thread() {};
	void start();
	int join();
	
	virtual int run() = 0;
	void signal_stop();
	void interrupt();
};

#endif /*THREAD_H_*/
