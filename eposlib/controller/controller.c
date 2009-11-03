/*      BlueBotics ERA-5/1 controller
 *
 *      Ralf Kaestner    ralf.kaestner@gmail.com
 *      Last change:     9.5.2008
 */

#include <stdlib.h>

#include "controller.h"
#include "timer.h"

era_thread_t era_controller_thread;

int era_controller_start(
  const era_arm_velocity_t* arm_velocities,
  const double* timestamps,
  int num_velocities) {
  era_controller_argument_t* arguments =
    malloc(sizeof(era_controller_argument_t));
  arguments->arm_velocities = arm_velocities;
  arguments->timestamps = timestamps;
  arguments->num_velocities = num_velocities;

  return era_thread_start(&era_controller_thread, era_controller_run,
    arguments, 0.0);
}

void era_controller_exit(void) {
  era_thread_exit(&era_controller_thread, 1);
}

void* era_controller_run(void* arg) {
  era_controller_argument_t* arguments = arg;
  int i;
  double start_time;

  era_timer_start(&start_time);

  for (i = 0; i < arguments->num_velocities; i++) {
    era_set_configuration(0, &arguments->arm_velocities[i]);

    era_timer_wait(start_time, 1.0/arguments->timestamps[i]);
  }

  return 0;
}
