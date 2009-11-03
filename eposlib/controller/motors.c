/*      Interfacing kinematic system model for BlueBotics ERA-5/1
 *      with EPOS control library
 *
 *      Fritz Stoeckli   stfritz@ethz.ch
 *      Last change:     7.5.2008
 */

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include <epos.h>

#include "motors.h"
#include "errors.h"

#define ERA_MOTORS_POLL_INTERVAL 100

#define ERA_MOTORS_TEST_LIMIT_SWITCHES 0x0003

const era_motor_configuration_t era_motor_tiks_per_revolution = {
  .shoulder_yaw = 2000,
  .shoulder_roll = 2000,
  .shoulder_pitch = 2000,
  .ellbow_pitch = 2000,
  .tool_roll = 2000,
  .tool_opening = 2000,
};

const era_motor_configuration_t era_motor_current_limit = {
  .shoulder_yaw = 2500,
  .shoulder_roll = 2000,
  .shoulder_pitch = 2000,
  .ellbow_pitch = 2000,
  .tool_roll = 300,
  .tool_opening = 300,
};

const era_motor_configuration_t era_motor_homing_method = {
  .shoulder_yaw = ERA_MOTORS_HOMING_METHOD_SENSORS,
  .shoulder_roll = ERA_MOTORS_HOMING_METHOD_SENSORS,
  .shoulder_pitch = ERA_MOTORS_HOMING_METHOD_SENSORS,
  .ellbow_pitch = ERA_MOTORS_HOMING_METHOD_SENSORS,
  .tool_roll = ERA_MOTORS_HOMING_METHOD_SENSORS,
  .tool_opening = ERA_MOTORS_HOMING_METHOD_CURRENT,
};

const era_motor_configuration_t era_motor_homing_current_threshold = {
  .shoulder_yaw = 1700,
  .shoulder_roll = 1200,
  .shoulder_pitch = 1400,
  .ellbow_pitch = 1400,
  .tool_roll = 150,
  .tool_opening = 100,
};

const era_motor_transmission_t era_motor_transmission = {
  .shoulder_yaw = -0.02*0.108695652,
  .shoulder_roll = -0.02*0.119047619,
  .shoulder_pitch = 0.02*0.119047619,
  .ellbow_pitch = 0.02*0.129032258,
  .tool_roll = -0.005*1.0,
  .tool_opening = -0.01*1.0,
};

void era_init_motor_configuration(
  era_motor_configuration_t* motor_configuration) {
  int i;

  double* conf = (double*)motor_configuration;

  for (i = 0; i < sizeof(era_motor_configuration_t)/sizeof(int); i++)
    conf[i] = 0;
}

void era_init_motor_velocity(
  era_motor_velocity_t* motor_velocity) {
  int i;

  double* vel = (double*)motor_velocity;

  for (i = 0; i < sizeof(era_motor_velocity_t)/sizeof(int); i++)
    vel[i] = 0;
}

void era_print_motor_configuration(
  FILE* stream,
  const era_motor_configuration_t* motor_configuration) {
  fprintf(stream, "%14s: % 6d tiks\n",
    "shoulder_yaw", motor_configuration->shoulder_yaw);
  fprintf(stream, "%14s: % 6d tiks\n",
    "shoulder_roll", motor_configuration->shoulder_roll);
  fprintf(stream, "%14s: % 6d tiks\n",
    "shoulder_pitch", motor_configuration->shoulder_pitch);
  fprintf(stream, "%14s: % 6d tiks\n",
    "ellbow_pitch", motor_configuration->ellbow_pitch);
  fprintf(stream, "%14s: % 6d tiks\n",
    "tool_roll", motor_configuration->tool_roll);
  fprintf(stream, "%14s: % 6d tiks\n",
    "tool_opening", motor_configuration->tool_opening);
}

void era_print_motor_velocity(
  FILE* stream,
  const era_motor_velocity_t* motor_velocity) {
  fprintf(stream, "%14s: % 6d rpm\n",
    "shoulder_yaw", motor_velocity->shoulder_yaw);
  fprintf(stream, "%14s: % 6d rpm\n",
    "shoulder_roll", motor_velocity->shoulder_roll);
  fprintf(stream, "%14s: % 6d rpm\n",
    "shoulder_pitch", motor_velocity->shoulder_pitch);
  fprintf(stream, "%14s: % 6d rpm\n",
    "ellbow_pitch", motor_velocity->ellbow_pitch);
  fprintf(stream, "%14s: % 6d rpm\n",
    "tool_roll", motor_velocity->tool_roll);
  fprintf(stream, "%14s: % 6d rpm\n",
    "tool_opening", motor_velocity->tool_opening);
}

void era_arm_to_motor(
  const era_arm_configuration_t* arm_configuration,
  const era_arm_velocity_t* arm_velocity,
  era_motor_configuration_t* motor_configuration,
  era_motor_velocity_t* motor_velocity) {
  int i;

  double* arm_conf = (double*)arm_configuration;
  int* motor_conf = (int*)motor_configuration;
  double* arm_vel = (double*)arm_velocity;
  int* motor_vel = (int*)motor_velocity;

  int* tiks_per_revolution = (int*)&era_motor_tiks_per_revolution;
  double* trans = (double*)&era_motor_transmission;

  for (i = 0; i < sizeof(era_motor_configuration_t)/sizeof(int); i++) {
    if (arm_conf && motor_conf)
      motor_conf[i] = arm_conf[i]/trans[i]*tiks_per_revolution[i]/(2*M_PI);

    if (arm_vel && motor_vel)
      motor_vel[i] = arm_vel[i]/trans[i]*60/(2*M_PI);
  }
}

void era_motor_to_arm(
  const era_motor_configuration_t* motor_configuration,
  const era_motor_velocity_t* motor_velocity,
  era_arm_configuration_t* arm_configuration,
  era_arm_velocity_t* arm_velocity) {
  int i;

  int* motor_conf = (int*)motor_configuration;
  double* arm_conf = (double*)arm_configuration;
  int* motor_vel = (int*)motor_velocity;
  double* arm_vel = (double*)arm_velocity;

  int* tiks_per_revolution = (int*)&era_motor_tiks_per_revolution;
  double* trans = (double*)&era_motor_transmission;

  for (i = 0; i < sizeof(era_arm_configuration_t)/sizeof(double); i++) {
    if (motor_conf && arm_conf)
      arm_conf[i] = motor_conf[i]*trans[i]*2*M_PI/tiks_per_revolution[i];

    if (arm_vel && motor_vel)
      arm_vel[i] = motor_vel[i]*trans[i]*2*M_PI/60;
  }
}

void era_motors_init(
  const char* dev) {
  int id;

  signal(SIGINT, era_motors_signaled);

  int* current_limit = (int*)&era_motor_current_limit;

  can_init(dev);

  for (id = 1; id < 7; id++) {
    epos_fault_reset(id);
    epos_set_output_current_limit(id, current_limit[id-1]);
  }
}

void era_motors_close(void) {
  int id;

  for (id = 1; id < 7; id++) epos_shutdown(id);

  can_close();
}

void era_motors_signaled(
  int signal) {
  if (signal == SIGINT) {
    era_motors_stop();

    era_motors_close();
  }

  exit(signal);
}

void era_motors_wait(
  int condition) {
  int id;
  short status = 0;

  while (!status) {
    status = 0xFFFF;

    for (id = 1; id < 7; id++) {
      if (condition == ERA_MOTORS_WAIT_STOP) {
        epos_get_actual_velocity(id);
        status &= (epos_read.node[id-1].actual_velocity == 0);
      }
      else {
        epos_get_statusword(id);
        status &= condition & epos_read.node[id-1].status;
      }
    }

    if (!status) usleep(ERA_MOTORS_POLL_INTERVAL);
  }
}

int era_motors_set_mode(
  int operation_mode) {
  int id;

  if ((operation_mode != ERA_MOTORS_OPERATION_MODE_POSITION) &&
    (operation_mode != ERA_MOTORS_OPERATION_MODE_VELOCITY))
    return ERA_ERROR_UNSUPPORTED_OPERATION_MODE;

  for (id = 1; id < 7; id++) {
    epos_shutdown(id);
    epos_enable_operation(id);

    if (operation_mode == ERA_MOTORS_OPERATION_MODE_POSITION) {
      epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_PROFILE_POSITION);
    }
    else if (operation_mode == ERA_MOTORS_OPERATION_MODE_VELOCITY) {
      epos_set_velocity_mode_setting_value(id, 0);
      epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_VELOCITY);
    }

    epos_shutdown(id);
    epos_enable_operation(id);
  }

  return ERA_ERROR_NONE;
}

int era_motors_home(
  const era_motor_velocity_t* motor_homing_velocity,
  const era_motor_configuration_t* motor_limit,
  const era_motor_configuration_t* motor_home) {
  int id;

  int* limit = (int*)motor_limit;
  int* home = (int*)motor_home;
  int* homing_method = (int*)&era_motor_homing_method;
  int* current_threshold = (int*)&era_motor_homing_current_threshold;
  int* homing_velocity = (int*)motor_homing_velocity;
  double* transmission = (double*)&era_motor_transmission;

  /* Check for limit switch states */
  for (id = 1; id < 7; id++) {
    epos_get_digital_input(id);

    if (epos_read.node[id-1].digital_input & ERA_MOTORS_TEST_LIMIT_SWITCHES)
      return ERA_ERROR_INVALID_INITIAL_CONFIGURATION;
  }

  /* Begin homing */
  for (id = 1; id < 7; id++) {
    if (homing_velocity[id-1] != 0) {
      epos_shutdown(id);
      epos_enable_operation(id);
      epos_set_mode_of_operation(id, EPOS_OPERATION_MODE_HOMING);
      epos_shutdown(id);
      epos_enable_operation(id);

      if (homing_method[id-1] == ERA_MOTORS_HOMING_METHOD_SENSORS) {
        if (homing_velocity[id-1] < 0) epos_set_homing_method(id,
          EPOS_HOMING_METHOD_NEGATIVE_LIMIT_SWITCH);
        else epos_set_homing_method(id,
          EPOS_HOMING_METHOD_POSITIVE_LIMIT_SWITCH);
      }
      else if (homing_method[id-1] == ERA_MOTORS_HOMING_METHOD_CURRENT) {
        if (homing_velocity[id-1] < 0) epos_set_homing_method(id,
          EPOS_HOMING_METHOD_NEGATIVE_CURRENT_THRESHOLD);
        else epos_set_homing_method(id,
          EPOS_HOMING_METHOD_POSITIVE_CURRENT_THRESHOLD);

        epos_set_homing_current_threshold(id, current_threshold[id-1]);
      }

      epos_set_home_position(id, home[id-1]);
      epos_set_home_offset(id, abs(limit[id-1]-home[id-1]));
      epos_set_homing_speed_switch_search(id, abs(homing_velocity[id-1]));
      epos_set_homing_speed_zero_search(id, abs(homing_velocity[id-1]));

      epos_start_homing_operation(id);
    }
  }

  return ERA_ERROR_NONE;
}

void era_motors_get_configuration(
  era_motor_configuration_t* motor_configuration,
  era_motor_velocity_t* motor_velocity) {
  int id;

  int* motor_conf = (int*)motor_configuration;
  int* motor_vel = (int*)motor_velocity;

  for (id = 1; id < 7; ++id) {
    if (motor_configuration) {
      epos_get_actual_position(id);
      motor_conf[id-1] = epos_read.node[id-1].actual_position;
    }

    if (motor_velocity) {
      epos_get_actual_velocity(id);
      motor_vel[id-1] = epos_read.node[id-1].actual_velocity;
    }
  }
}

int era_motors_set_configuration(
  const era_motor_configuration_t* motor_configuration,
  const era_motor_velocity_t* motor_velocity) {
  int id;

  int* motor_conf = (int*)motor_configuration;
  int* motor_vel = (int*)motor_velocity;

  if (motor_configuration) {
    era_motors_set_mode(ERA_MOTORS_OPERATION_MODE_POSITION);

    for (id = 1; id < 7; ++id) {
      epos_set_profile_velocity(id, abs(motor_vel[id-1]));
      epos_set_target_position(id, motor_conf[id-1]);
    }

    for (id = 1; id < 7; ++id) epos_activate_position(id);
  }
  else {
    era_motors_set_mode(ERA_MOTORS_OPERATION_MODE_VELOCITY);

    for (id = 1; id < 7; id++)
      epos_set_velocity_mode_setting_value(id, motor_vel[id-1]);
  }

  return ERA_ERROR_NONE;
}

void era_motors_stop(void) {
  int id;

  for (id = 1; id < 7; id++) epos_quick_stop(id);
}
