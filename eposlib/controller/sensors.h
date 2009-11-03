/*	Header-file for
 *	BlueBotics ERA-5/1 sensor data acquisition
 *
 * 	Ralf Kaestner    ralf.kaestner@gmail.com
 * 	Last change:     9.5.2008
 */

#ifndef _SENSORS_H
#define _SENSORS_H

#include "thread.h"
#include "kinematics.h"
#include "velocity.h"

/** \file
  * \brief The BlueBotics ERA-5/1 sensor data acquisition
  *
  * The sensor data acquisition thread for the BlueBotics ERA-5/1.
  */

/** \brief Callback handler definition for sensor reading updates */
typedef void (*era_read_handler_t)(
  const era_arm_configuration_t* configuration,
  const era_arm_velocity_t* velocity,
  double actual_frequency);

/** \brief Structure defining the sensor arguments */
typedef struct {
  era_read_handler_t handler;   //!< The handler receiving the readings.
  double timestamp;             //!< The timestamp of the last acquisition [s].
} era_sensors_argument_t;

/** \brief Static variable containing the sensor data acquisition thread */
extern era_thread_t era_sensors_thread;

/** \brief Start the sensor data acquisition
  * Performs a timed read of arm configurations from the motors.
  * \param[in] handler The callback handler that will receive the sensor
  *   reading updates.
  * \param[in] frequency The sensor data acquisition frequency.
  * \return The resulting error code.
  */
int era_sensors_start(
  era_read_handler_t handler,
  double frequency);

/** \brief Exit the sensor data acquisition
  */
void era_sensors_exit(void);

/** \brief Run the sensor data acquisition
  * This function is run within the sensor data acquisition thread and
  * should never be called directly.
  * \param[in] arg The arguments passed to the sensor data acquisition thread.
  * \return The result of the sensor data acquisition thread.
  */
void* era_sensors_run(void* arg);

#endif
