/*      BlueBotics ERA-5/1 controller
 *
 *      Ralf Kaestner    ralf.kaestner@gmail.com
 *      Last change:     9.5.2008
 */

#include <sys/time.h>
#include <pthread.h>

#include "timer.h"
#include "errors.h"

void era_timer_start(
  double* timestamp) {
  struct timeval time;
  double million = 1000000.0;

  gettimeofday(&time, 0);

  *timestamp = time.tv_sec+time.tv_usec/million;
}

double era_timer_stop(
  double timestamp) {
  struct timeval time;
  double million = 1000000.0;

  gettimeofday(&time, 0);

  return time.tv_sec+time.tv_usec/million-timestamp;
}

double era_timer_get_frequency(
  double timestamp) {
  return 1.0/era_timer_stop(timestamp);
}

int era_timer_wait(
  double timestamp,
  double frequency) {
  if (frequency <= 0.0) return ERA_ERROR_TIMER_FAULT;

  return era_timer_sleep(1.0/frequency-era_timer_stop(timestamp));
}

int era_timer_sleep(
  double seconds) {
  if (seconds < 0.0) return ERA_ERROR_TIMER_FAULT;

  usleep((int)(seconds*1000000.0));

  return ERA_ERROR_NONE;
}
