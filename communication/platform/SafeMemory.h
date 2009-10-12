#ifndef SAFEMEMORY_H_
#define SAFEMEMORY_H_

#include "Exception.h"

#include <pthread.h>

#include <string>

/**
 * Wraps a variable to make it thread-safe. Read and write access is protected
 * by a single mutex. No two threads can access the variable using any of the
 * read() or write() functions at the same time.
 *
 * The encapsulated type needs to have a default constructor and a copy
 * constructor.
 */
template <typename T> class SafeMemory
{
// data members
private:
	T content;
	pthread_mutex_t mutex;

// methods
public:
	SafeMemory()
	{
		pthread_mutex_init(&mutex, NULL);
	}

	~SafeMemory()
	{
		pthread_mutex_destroy(&mutex);
	}
	
	/**
	 * Read the value atomically.
	 */
	T read()
	{
		if(pthread_mutex_lock(&mutex) != 0) {
			throw(Exception("locking SafeMemory mutex for reading"));
		}
		T ret = content;
		if(pthread_mutex_unlock(&mutex) != 0) {
			throw(Exception("unlocking SafeMemory mutex for reading"));
		}
		return ret;
	}
	
	/**
	 * Atomically read the current value and then overwrite it.
	 *
	 * @param s	new value to be written
	 * @return	value of memory before overwriting
	 */
	T read_write(const T &s)
	{
		if(pthread_mutex_lock(&mutex) != 0) {
			throw(Exception("locking SafeMemory mutex for read/writing"));
		}
		T ret = content;
		content = s;
		if(pthread_mutex_unlock(&mutex) != 0) {
			throw(Exception("unlocking SafeMemory mutex for read/writing"));
		}
		return ret;
	}
	
	/**
	 * Write the value atomically.
	 *
	 * @param v;
	 */
	void write(const T &v)
	{
		if(pthread_mutex_lock(&mutex) != 0) {
			throw(Exception("locking SafeMemory mutex for writing"));
		}
		content = v;
		if(pthread_mutex_unlock(&mutex) != 0) {
			throw(Exception("unlocking SafeMemory mutex for writing"));
		}
	}
};

#endif /*SAFEMEMORY_H_*/
