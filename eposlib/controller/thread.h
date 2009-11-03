/*	Header-file for
 *	BlueBotics ERA-5/1 threading
 *
 * 	Ralf Kaestner    ralf.kaestner@gmail.com
 * 	Last change:     9.5.2008
 */

#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>

/** \file
  * \brief Thread handling
  *
  * Thread handling for the BlueBotics ERA-5/1.
  */

/** \brief Structure defining the thread context */
typedef struct {
  pthread_t thread;         //!< The thread handle.
  void* (*routine)(void*);  //!< The thread routine.
  void* arg;                //!< The thread routine argument.
  pthread_mutex_t mutex;    //!< The thread mutex.
  double frequency;         //!< The thread cycle frequency in [Hz].
  double start_time;        //!< The thread start timestamp.
  int exit_request;         //!< Flag signaling a pending exit request.
} era_thread_t;

/** \brief Start a thread
  * \param[in] thread The thread to be started.
  * \param[in] thread_routine The thread routine that will be executed
  *   within the thread.
  * \param[in] thread_arg The argument to be passed on to the thread
  *   routine. The memory should be allocated by the caller and will be
  *   freed after thread termination.
  * \param[in] frequency The thread cycle frequency. If the frequency is 0,
  *   the thread routine will be executed once.
  * \return The resulting error code.
  */
int era_thread_start(
  era_thread_t* thread,
  void* (*thread_routine)(void*),
  void* thread_arg,
  double frequency);

/** \brief Exit a thread
  * \param[in] thread The thread to be terminated.
  * \param[in] wait If 0, return instantly, wait for thread termination
  *   otherwise.
  */
void era_thread_exit(
  era_thread_t* thread,
  int wait);

/** \brief Run the thread
  * This function is run within the thread and should never be called
  * directly.
  * \param[in] arg The arguments passed to the thread.
  * \return The result of the thread.
  */
void* era_thread_run(void* arg);

/** \brief Test thread for a pending exit request
  * \param[in] thread The thread to be tested for a pending exit request.
  * \return 1 if an exit request is pending, 0 otherwise.
  */
int era_test_thread_exit(
  era_thread_t* thread);

/** \brief Wait for thread termination
  * \param[in] thread The thread to wait for.
  */
void era_thread_wait_exit(
  era_thread_t* thread);

#endif
