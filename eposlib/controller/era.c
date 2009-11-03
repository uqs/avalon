/*      BlueBotics ERA-5/1 control
 *
 *      Ralf Kaestner    ralf.kaestner@gmail.com
 *      Last change:     9.5.2008
 */

#include <pthread.h>
#include <math.h>

#include <stdlib.h>
#include <string.h>

#include "era.h"
#include "motors.h"
#include "errors.h"

#define sqr(x) x*x

era_arm_configuration_t era_home = {
  .shoulder_yaw = 0.0*M_PI/180.0,
  .shoulder_roll = 10.0*M_PI/180.0,
  .shoulder_pitch = 0.0*M_PI/180.0,
  .ellbow_pitch = 25.0*M_PI/180.0,
  .tool_roll = -10.0*M_PI/180.0,
  .tool_opening = 0.0*M_PI/180.0,
};

const era_arm_velocity_t era_homing_velocity = {
  .shoulder_yaw = 3.0*M_PI/180.0,
  .shoulder_roll = -3.0*M_PI/180.0,
  .shoulder_pitch = -3.0*M_PI/180.0,
  .ellbow_pitch = -3.0*M_PI/180.0,
  .tool_roll = -15.0*M_PI/180.0,
  .tool_opening = -10.0*M_PI/180.0,
};

pthread_mutex_t era_mutex;

void era_print_configuration(
  FILE* stream,
  const era_arm_configuration_t* arm_configuration,
  const era_arm_velocity_t* arm_velocity) {
  era_tool_configuration_t tool_configuration;

  era_forward_kinematics(arm_configuration, &tool_configuration);

  fprintf(stream, "%s %23s  %29s  %21s\n",
    "ARM", "CONFIGURATION",
    "VELOCITY",
    "TOOL CONFIGURATION");

  fprintf(stream, "%14s: % 7.2f deg  %14s: % 7.2f deg/s  %7s: % 7.4f m\n",
    "shoulder_yaw", arm_configuration->shoulder_yaw*180/M_PI,
    "shoulder_yaw", arm_velocity->shoulder_yaw*180/M_PI,
    "x", tool_configuration.x);
  fprintf(stream, "%14s: % 7.2f deg  %14s: % 7.2f deg/s  %7s: % 7.4f m\n",
    "shoulder_roll", arm_configuration->shoulder_roll*180/M_PI,
    "shoulder_roll", arm_velocity->shoulder_roll*180/M_PI,
    "y", tool_configuration.y);
  fprintf(stream, "%14s: % 7.2f deg  %14s: % 7.2f deg/s  %7s: % 7.4f m\n",
    "shoulder_pitch", arm_configuration->shoulder_pitch*180/M_PI,
    "shoulder_pitch", arm_velocity->shoulder_pitch*180/M_PI,
    "z", tool_configuration.z);
  fprintf(stream, "%14s: % 7.2f deg  %14s: % 7.2f deg/s  %7s: % 7.2f deg\n",
    "ellbow_pitch", arm_configuration->ellbow_pitch*180/M_PI,
    "ellbow_pitch", arm_velocity->ellbow_pitch*180/M_PI,
    "yaw", tool_configuration.yaw*180/M_PI);
  fprintf(stream, "%14s: % 7.2f deg  %14s: % 7.2f deg/s  %7s: % 7.2f deg\n",
    "tool_roll", arm_configuration->tool_roll*180/M_PI,
    "tool_roll", arm_velocity->tool_roll*180/M_PI,
    "roll", tool_configuration.roll*180/M_PI);
  fprintf(stream, "%14s: % 7.2f deg  %14s: % 7.2f deg/s  %7s: % 7.2f deg\n",
    "tool_opening", arm_configuration->tool_opening*180/M_PI,
    "tool_opening", arm_velocity->tool_opening*180/M_PI,
    "opening", tool_configuration.opening*180/M_PI);
}

void era_print(
  FILE* stream) {
  era_arm_configuration_t arm_configuration;
  era_arm_velocity_t arm_velocity;

  era_get_configuration(&arm_configuration, &arm_velocity);

  era_print_configuration(stream, &arm_configuration, &arm_velocity);
}

int era_init(
  const char* dev) {
  int i;

  pthread_mutex_init(&era_mutex, NULL);

  era_motors_init(dev);

  era_arm_configuration_t arm_limit;
  double* lim = (double*)&arm_limit;
  double* min = (double*)&era_arm_configuration_min;
  double* max = (double*)&era_arm_configuration_max;
  double* vel = (double*)&era_homing_velocity;

  for (i = 0; i < sizeof(era_arm_configuration_t)/sizeof(double); i++) {
    if (vel[i] < 0.0) lim[i] = min[i];
    else lim[i] = max[i];
  }

  era_motor_configuration_t motor_limit;
  era_motor_configuration_t motor_home;
  era_motor_velocity_t motor_homing_velocity;
  era_arm_to_motor(&arm_limit, &era_homing_velocity, &motor_limit,
    &motor_homing_velocity);
  era_arm_to_motor(&era_home, 0, &motor_home, 0);

  int result = era_motors_home(&motor_homing_velocity, &motor_limit,
    &motor_home);

  if (!result) era_motors_wait(ERA_MOTORS_WAIT_HOME_ATTAINED);

  return result;
}

void era_close() {
  era_motors_close();

  pthread_mutex_destroy(&era_mutex);
}

int era_read(
  era_read_handler_t handler,
  double frequency) {
  int result = era_sensors_start(handler, frequency);

  if (!result) era_thread_wait_exit(&era_sensors_thread);

  return result;
}

void era_get_configuration(
  era_arm_configuration_t* configuration,
  era_arm_velocity_t* velocity) {
  era_motor_configuration_t motor_configuration;
  era_motor_velocity_t motor_velocity;

  pthread_mutex_lock(&era_mutex);

  if (configuration)
    era_motors_get_configuration(&motor_configuration, 0);
  if (velocity)
    era_motors_get_configuration(0, &motor_velocity);

  pthread_mutex_unlock(&era_mutex);

  era_motor_to_arm(&motor_configuration, &motor_velocity,
    configuration, velocity);
}

int era_set_configuration(
  const era_arm_configuration_t* configuration,
  const era_arm_velocity_t* velocity) {
  int result;

  result = era_test_arm_configuration_limits(configuration);
  if (!result) result = era_test_arm_velocity_limits(velocity);

  if (!result) {
    era_motor_configuration_t motor_configuration;
    era_motor_velocity_t motor_velocity;
    era_arm_to_motor(configuration, velocity, &motor_configuration,
      &motor_velocity);

    pthread_mutex_lock(&era_mutex);

    if (configuration)
      result = era_motors_set_configuration(&motor_configuration,
      &motor_velocity);
    else
      result = era_motors_set_configuration(0, &motor_velocity);

    pthread_mutex_unlock(&era_mutex);
  }

  return result;
}

int era_move(
  const era_arm_configuration_t* target,
  double velocity,
  int wait) {
  int result;

  era_arm_configuration_t current;
  era_arm_velocity_t arm_velocity;

  era_get_configuration(&current, 0);
  era_sync_arm_velocity(&current, target, velocity, &arm_velocity);

  result = era_set_configuration(target, &arm_velocity);

  if (!result && wait) era_motors_wait(ERA_MOTORS_WAIT_TARGET_REACHED);

  return result;
}

int era_move_home(
  double velocity,
  int wait) {
  return era_move(&era_home, velocity, wait);
}

int era_move_tool(
  const era_tool_configuration_t* target,
  double velocity,
  int wait) {
  era_arm_configuration_t arm_configuration;

  era_inverse_kinematics(target, &arm_configuration);

  return era_move(&arm_configuration, velocity, wait);
}

int era_move_trajectory(
  const era_arm_configuration_t* trajectory,
  const double* timestamps,
  int num_configurations) {
  int result;

  result = era_test_trajectory_limits(trajectory, num_configurations, 0);

  if (!result) {
    era_arm_velocity_t velocities[num_configurations];

    era_velocity_profile(trajectory, timestamps, num_configurations,
      velocities);

    result = era_test_velocity_profile_limits(velocities, num_configurations);

    if (!result) {
      result = era_move(&trajectory[0], ERA_DEFAULT_VELOCITY, 1);

      if (!result)
        result = era_move_velocity_profile(velocities, timestamps,
        num_configurations);
    }
  }

  return result;
}

int era_move_tool_trajectory(
  const era_tool_configuration_t* trajectory,
  const double* timestamps,
  int num_configurations) {
  era_arm_configuration_t arm_trajectory[num_configurations];

  era_trajectory_inverse_kinematics(trajectory, num_configurations,
    arm_trajectory);

  return era_move_trajectory(arm_trajectory, timestamps, num_configurations);
}

int era_move_velocity_profile(
  const era_arm_velocity_t* arm_velocities,
  const double* timestamps,
  int num_velocities) {
  int result =
    era_controller_start(arm_velocities, timestamps, num_velocities);

  if (!result) era_thread_wait_exit(&era_controller_thread);

  return result;
}

double era_get_configuration_error(
  const era_arm_configuration_t* target) {
  int i;
  double squared_error = 0.0;

  era_arm_configuration_t current;
  double* current_conf = (double*)&current;
  double* target_conf = (double*)target;

  era_get_configuration(&current, 0);

  for (i = 0; i < sizeof(era_arm_configuration_t)/sizeof(double); i++)
    squared_error += sqr(target_conf[i]-current_conf[i]);

  return sqrt(squared_error);
}
