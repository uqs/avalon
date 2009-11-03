
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include <epos.h>

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
