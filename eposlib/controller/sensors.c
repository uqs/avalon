/*      BlueBotics ERA-5/1 sensor data acquisition
 *
 *      Ralf Kaestner    ralf.kaestner@gmail.com
 *      Last change:     9.5.2008
 */

#include <stdlib.h>

#include "sensors.h"
#include "era.h"

era_thread_t era_sensors_thread;

int era_sensors_start(
  era_read_handler_t handler,
  double frequency) {
  era_sensors_argument_t* arguments = malloc(sizeof(era_sensors_argument_t));
  arguments->handler = handler;
  arguments->timestamp = 2.0;

  return era_thread_start(&era_sensors_thread, era_sensors_run,
    arguments, frequency);
}

void era_sensors_exit(void) {
  era_thread_exit(&era_sensors_thread, 1);
}

void* era_sensors_run(void* arg) {
  era_sensors_argument_t* arguments = arg;
  era_arm_configuration_t configuration;
  era_arm_velocity_t velocity;

  era_get_configuration(&configuration, &velocity);

  if (arguments->handler)
    arguments->handler(&configuration, &velocity,
    era_sensors_thread.frequency);

  return 0;
}
