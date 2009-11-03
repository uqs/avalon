/*	Header-file for
 *	Interfacing kinematic system model for BlueBotics ERA-5/1
 *      with EPOS control library
 *
 * 	Fritz Stoeckli   stfritz@ethz.ch
 * 	Last change:     7.5.2008
 */

#ifndef _MOTORS_H
#define _MOTORS_H

#include <trajectory.h>
#include <velocity_profile.h>

/** \file
  * \brief Interfacing kinematic system model with EPOS controller
  *
  * Providing a set of funcions to use EPOS control library with the
  * kinematic system model for BlueBotics ERA-5/1.
  */

/** \brief Operation modes of the motor controller
  */
#define ERA_MOTORS_OPERATION_MODE_POSITION 0
#define ERA_MOTORS_OPERATION_MODE_VELOCITY 1

/** \brief Homing methods of the motor controller
  */
#define ERA_MOTORS_HOMING_METHOD_SENSORS 0
#define ERA_MOTORS_HOMING_METHOD_CURRENT 1

/** \brief Wait conditions of the motor controller
  */
#define ERA_MOTORS_WAIT_STOP 0x0000
#define ERA_MOTORS_WAIT_FAULT 0x0008
#define ERA_MOTORS_WAIT_TARGET_REACHED 0x0408
#define ERA_MOTORS_WAIT_HOME_ATTAINED 0x1008

/** \brief Structure defining the motor configuration */
typedef struct {
  int shoulder_yaw;    //!< The shoulder's yaw increments [tiks].
  int shoulder_roll;   //!< The shoulder's roll increments [tiks].
  int shoulder_pitch;  //!< The shoulder's pitch increments [tiks].
  int ellbow_pitch;    //!< The ellbow's pitch increments [tiks].
  int tool_roll;       //!< The tool's roll increments [tiks].
  int tool_opening;    //!< The tool's opening increments [tiks].
} era_motor_configuration_t;

/** \brief Structure defining the motor velocity
  * All components are given in [rpm].
  */
typedef era_motor_configuration_t era_motor_velocity_t;

/** \brief Structure defining the motor velocity
  * All components are given in [rad/tiks].
  */
typedef era_arm_configuration_t era_motor_transmission_t;

/** \brief Constant defining the number of motor tiks per revolution */
extern const era_motor_configuration_t era_motor_tiks_per_revolution;
/** \brief Constant defining the motor current limit */
extern const era_motor_configuration_t era_motor_current_limit;

/** \brief Constant defining the motor homing methods */
extern const era_motor_configuration_t era_motor_homing_method;
/** \brief Constant defining the motor home current threshold */
extern const era_motor_configuration_t era_motor_homing_current_threshold;

/** \brief Constant defining the motor transmission */
extern const era_motor_transmission_t era_motor_transmission;

/** \brief Initialize a motor configuration
  * \param[in] motor_configuration The motor configuration to be initialized
  *   with 0.
  */
void era_init_motor_configuration(
  era_motor_configuration_t* motor_configuration);

/** \brief Initialize a motor velocity
  * \param[in] motor_velocity The motor velocity to be initialized  with 0.
  */
void era_init_motor_velocity(
  era_motor_velocity_t* motor_velocity);

/** \brief Print a motor configuration
  * \param[in] stream The output stream that will be used for printing the
  *   motor configuration.
  * \param[in] motor_configuration The motor configuration that will be printed.
  */
void era_print_motor_configuration(
  FILE* stream,
  const era_motor_configuration_t* motor_configuration);

/** \brief Print a motor velocity
  * \param[in] stream The output stream that will be used for printing the
  *   motor velocity.
  * \param[in] motor_velocity The motor velocity that will be printed.
  */
void era_print_motor_velocity(
  FILE* stream,
  const era_motor_velocity_t* motor_velocity);

/** \brief Convert arm configuration and velocity into motor space
  * \param[in] arm_configuration The arm configuration to be converted
  *   into motor space. Can be null.
  * \param[in] arm_velocity The arm velocity to be converted
  *   into motor space. Can be null.
  * \param[out] motor_configuration The motor configuration that results
  *   from the conversion. Can be null.
  * \param[in] motor_velocity The motor velocity that results
  *   from the conversion. Can be null.
  */
void era_arm_to_motor(
  const era_arm_configuration_t* arm_configuration,
  const era_arm_velocity_t* arm_velocity,
  era_motor_configuration_t* motor_configuration,
  era_motor_velocity_t* motor_velocity);

/** \brief Convert motor configuration and velocity into arm space
  * \param[in] motor_configuration The motor configuration to be converted
  *   into arm space. Can be null.
  * \param[in] motor_velocity The motor velocity to be converted
  *   into arm space. Can be null.
  * \param[out] arm_configuration The arm configuration that results
  *   from the conversion. Can be null.
  * \param[out] arm_velocity The arm velocity that results
  *   from the conversion. Can be null.
  */
void era_motor_to_arm(
  const era_motor_configuration_t* motor_configuration,
  const era_motor_velocity_t* motor_velocity,
  era_arm_configuration_t* arm_configuration,
  era_arm_velocity_t* arm_velocity);

/** \brief Initialize the motor communication
  * Initializes CAN communication to the motors.
  * \param[in] dev The character device the motor controllers are attached to.
  */
void era_motors_init(
  const char* dev);

/** \brief Closes the motor communication
  * Closes CAN communication to the motors.
  */
void era_motors_close(void);

/** \brief Termination signal handler
  * This handler performs a motor quick stop on program termination.
  * \param[in] signal The caught signal, should be SIGINT.
  */
void era_motors_signaled(
  int signal);

/** \brief Wait for a motor condition
  * \param[in] condition The wait condition. Possible values are
  *   ERA_MOTORS_WAIT_STOP, ERA_MOTORS_WAIT_FAULT,
  *   ERA_MOTORS_WAIT_TARGET_REACHED, and ERA_MOTORS_WAIT_HOME_ATTAINED.
  */
void era_motors_wait(
  int condition);

/** \brief Set the specified motor operation mode
  * \param[in] operation_mode The motor operation mode to be set. Possible
  *   values are ERA_OPERATION_MODE_POSITION and ERA_OPERATION_MODE_VELOCITY.
  * \return The resulting error code.
  */
int era_motors_set_mode(
  int operation_mode);

/** \brief Perform motor homing
  * Find predefined limit switches of the motors and return to the
  * predefined motor home offset.
  * \param[in] motor_homing_velocity The motor velocity that will be
  *   set for searching the limit switches and encoder pulses. For positive
  *   motor velocity components, the positive limit switch will be searched
  *   and vice versa.
  * \param[in] motor_limit The motor configuration that represents the
  *   motor configuration space limit.
  * \param[in] motor_home The motor home configuration that will be assumed
  *   once the home position has been attained.
  * \return The resulting error code. If any of the limit switches is in
  *   active state, ERA_ERROR_INVALID_INITIAL_CONFIGURATION will be returned.
  *   In fact, for the ERA-5/1, the limit switch activities are ambiguous
  *   since there exists only one switch per joint. This may result in an
  *   unsafe behavior of the homing operation, eventually causing damage to
  *   the arm.
  */
int era_motors_home(
  const era_motor_velocity_t* motor_homing_velocity,
  const era_motor_configuration_t* motor_limit,
  const era_motor_configuration_t* motor_home);

/** \brief Get motor configuration and velocity
  * \param[out] motor_configuration The current motor configuration.
  *   Can be null.
  * \param[out] motor_velocity The current motor velocity. Can be null.
  */
void era_motors_get_configuration(
  era_motor_configuration_t* motor_configuration,
  era_motor_velocity_t* motor_velocity);

/** \brief Set motor configuration and velocity
  * \param[in] motor_configuration The motor configuration to be set. If null,
  *   the motors will be switched into velocity mode. Otherwise, the motors
  *   will be switched into position mode and the motor configuration will
  *   set as target position.
  * \param[in] motor_velocity The motor profile velocity to be set.
  *   Cannot be null.
  * \return The resulting error code.
  */
int era_motors_set_configuration(
  const era_motor_configuration_t* motor_configuration,
  const era_motor_velocity_t* motor_velocity);

/** \brief Stop the motors
  * Emits a quick stop message to the motors.
  */
void era_motors_stop(void);

#endif
