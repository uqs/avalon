/*      BlueBotics ERA-5/1 controller
 *
 *      Ralf Kaestner    ralf.kaestner@gmail.com
 *      Last change:     9.5.2008
 */

#include "thread.h"
#include "errors.h"

int era_thread_start(
  era_thread_t* thread,
  void* (*thread_routine)(void*),
  void* thread_arg,
  double frequency) {
  thread->routine = thread_routine;
  thread->arg = thread_arg;
  thread->frequency = frequency;
  thread->start_time = 0.0;
  thread->exit_request = 0;

  if (pthread_create(&thread->thread, NULL, era_thread_run, thread))
    return ERA_ERROR_THREAD_CREATE;
  else
    return ERA_ERROR_NONE;
}

void era_thread_exit(
  era_thread_t* thread,
  int wait) {
  pthread_mutex_lock(&thread->mutex);
  thread->exit_request = 1;
  pthread_mutex_unlock(&thread->mutex);

  if (wait) era_thread_wait_exit(thread);
}

void* era_thread_run(void* arg) {
  era_thread_t* thread = arg;
  void* result;

  pthread_mutex_init(&thread->mutex, NULL);
  era_timer_start(&thread->start_time);

  if (thread->frequency > 0.0) {
    while (!era_test_thread_exit(thread)) {
      double timestamp;
      era_timer_start(&timestamp);

      result = thread->routine(thread->arg);

      era_timer_wait(timestamp, thread->frequency);
    }
  }
  else result = thread->routine(thread->arg);

  if (thread->arg) free(thread->arg);
  pthread_mutex_destroy(&thread->mutex);

  return result;
}

int era_test_thread_exit(
  era_thread_t* thread) {
  int result = 0;

  pthread_mutex_lock(&thread->mutex);
  result = thread->exit_request;
  pthread_mutex_unlock(&thread->mutex);

  return result;
}

void era_thread_wait_exit(
  era_thread_t* thread) {
  pthread_join(thread->thread, NULL);
}
