

#include "Thread.h"
#include "Exception.h"

#include <signal.h>

#include <sstream>

extern "C" void thread_signal_handler(int signal) {
}

extern "C" void* start_thread(void *object) {
	// block all signals except SIGUSR1
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);	
	pthread_sigmask(SIG_SETMASK, &mask, NULL);

	signal(SIGUSR1, thread_signal_handler);	
	return (void*) ((Thread*)object)->run();
}

Thread::Thread() : thread_id(0), done(false)
{
}


/**
 * Creates a new thread which runs the run() method. The new thread will have
 * all signals masked except for SIGUSR1.
 */
void Thread::start()
{
	if(thread_id != 0) {
		throw(Exception("thread has already been started"));
	}
	if(pthread_create(&thread_id, NULL, start_thread, (void *) this ) != 0) {
		thread_id = 0;
		throw(Exception("couldn't create thread"));	
	}
}

/**
 * "Interrupts" the thread by sending SIGUSR1.
 */
void Thread::interrupt()
{
	if(pthread_kill(thread_id, SIGUSR1) != 0) {
		throw(Exception("couldn't send signal to thread"));
	}
}

/**
 * Signals the thread that we would like it to stop by setting a flag and
 * interrupting it.
 */
void Thread::signal_stop()
{
	done = true;
	interrupt();
}

/**
 * Wait for the thread to exit.
 */
int Thread::join()
{
	void* ret;
	if(pthread_join(thread_id, &ret) != 0) {
		std::ostringstream os;
		os << "while joining thread " << + thread_id;
		throw(Exception(os.str()));
	}
	thread_id = 0;
	return (int)ret;
}
