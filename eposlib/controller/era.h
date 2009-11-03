/*	Header-file for
 *	BlueBotics ERA-5/1 control
 *
 * 	Ralf Kaestner    ralf.kaestner@gmail.com
 * 	Last change:     9.5.2008
 */

#ifndef _ERA_H
#define _ERA_H

#include "sensors.h"
#include "controller.h"

/** \file
  * \brief The BlueBotics ERA-5/1 control functions
  *
  * The high-level control functions for the BlueBotics ERA-5/1.
  */

/** \brief Default parameters
  */
#define ERA_DEFAULT_CONTROL_FREQUENCY 25.0
#define ERA_DEFAULT_VELOCITY 0.1

/** \brief Constant defining the homing velocity */
extern const era_arm_velocity_t era_homing_velocity;

/** \brief Structure holding the home configuration */
extern era_arm_configuration_t era_home;

/** \brief Print an arm and tool configuration
  * This function also calculates and prints the tool configuration for
  * the given arm configuration.
  * \param[in] stream The output stream that will be used for printing the
  *   current arm and tool configuration.
  * \param[in] arm_configuration The arm configuration that will be printed.
  * \param[in] arm_velocity The arm velocity that will be printed.
  */
void era_print_configuration(
  FILE* stream,
  const era_arm_configuration_t* arm_configuration,
  const era_arm_velocity_t* arm_velocity);

/** \brief Print the current arm and tool configuration
  * \param[in] stream The output stream that will be used for printing the
  *   current arm and tool configuration.
  */
void era_print(
  FILE* stream);

/** \brief Initialize communication with the arm and perform homing
  * \param[in] dev The character device the arm is attached to.
  * \return The resulting error code.
  */
int era_init(
  const char* dev);

/** \brief Close communication with the arm
  */
void era_close(void);

/** \brief Get the current arm configuration and velocity
  * \param[out] configuration The current arm configuration. Can be null.
  * \param[out] velocity The current arm velocity. Can be null.
  */
void era_get_configuration(
  era_arm_configuration_t* configuration,
  era_arm_velocity_t* velocity);

/** \brief Set arm configuration and velocity
  * \param[in] configuration The arm configuration to be set. Can be null.
  * \param[in] velocity The arm velocity to be set. Cannot be null.
  * \return The resulting error code.
  */
int era_set_configuration(
  const era_arm_configuration_t* configuration,
  const era_arm_velocity_t* velocity);

/** \brief Read the arm sensors to a callback handler
  * \param[in] handler The callback handler that will receive the sensor
  *   reading updates.
  * \param[in] frequency The sensor data acquisition frequency.
  * \return The resulting error code.
  */
int era_read(
  era_read_handler_t handler,
  double frequency);

/** \brief Move the arm to a specified configuration
  * \param[in] target The target arm configuration.
  * \param[in] velocity The velocity of the arm in the range of 0 to 1.
  * \param[in] wait If 0, return instantly, wait for completion of the move
  *   operation otherwise.
  * \return The resulting error code.
  */
int era_move(
  const era_arm_configuration_t* target,
  double velocity,
  int wait);

/** \brief Move the arm to the predefined home configuration
  * \param[in] velocity The velocity of the arm in the range of 0 to 1.
  * \param[in] wait If 0, return instantly, wait for completion of the move
  *   operation otherwise.
  * \return The resulting error code.
  */
int era_move_home(
  double velocity,
  int wait);

/** \brief Move the tool to a specified configuration
  * \param[in] target The target tool configuration.
  * \param[in] velocity The velocity of the arm in the range of 0 to 1.
  * \param[in] wait If 0, return instantly, wait for completion of the move
  *   operation otherwise.
  * \return The resulting error code.
  */
int era_move_tool(
  const era_tool_configuration_t* target,
  double velocity,
  int wait);

/** \brief Move the arm along a given trajectory
  * Initially, the arm will be positioned in the start configuration. The
  * controller will then execute a velocity profile.
  * \param[in] trajectory An array of arm configurations representing
  *   the arm trajectory.
  * \param[in] timestamps An array of absolute timestamps associated with
  *   the arm trajectory points [s].
  * \param[in] num_configurations The number of configurations contained
  *   in the arm trajectory.
  * \return The resulting error code.
  */
int era_move_trajectory(
  const era_arm_configuration_t* trajectory,
  const double* timestamps,
  int num_configurations);

/** \brief Move the tool along a given trajectory
  * Initially, the arm will be positioned in the start configuration. The
  * controller will then execute a velocity profile.
  * \param[in] trajectory An array of tool configurations representing
  *   the tool trajectory.
  * \param[in] timestamps An array of absolute timestamps associated with
  *   the tool trajectory points [s].
  * \param[in] num_configurations The number of configurations contained
  *   in the tool trajectory.
  * \return The resulting error code.
  */
int era_move_tool_trajectory(
  const era_tool_configuration_t* trajectory,
  const double* timestamps,
  int num_configurations);

/** \brief Execute a given velocity profile
  * Starts the controller on the given velocity profile.
  * \param[in] arm_velocities The arm velocities that will be fed to the
  *   motors.
  * \param[in] timestamps An array of absolute timestamps associated with
  *   the arm velocities [s].
  * \param[in] num_velocities The number of velocities contained in the
  *   profile.
  * \return The resulting error code.
  */
int era_move_velocity_profile(
  const era_arm_velocity_t* arm_velocities,
  const double* timestamps,
  int num_velocities);

/** \brief Evaluate the error for a specified target arm configuration
  * \param[in] target The target arm configuration for which the configuration
  *   error will be evaluated.
  * \return The square root of the squared elements of the configuration error.
  */
double era_get_configuration_error(
  const era_arm_configuration_t* target);

#endif
