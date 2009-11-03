/*	Header-file for
 *	BlueBotics ERA-5/1 controller
 *
 * 	Ralf Kaestner    ralf.kaestner@gmail.com
 * 	Last change:     9.5.2008
 */

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "velocity_profile.h"
#include "thread.h"

/** \file
  * \brief The BlueBotics ERA-5/1 controller
  *
  * The motion controller for the BlueBotics ERA-5/1.
  */

/** \brief Structure defining the controller arguments */
typedef struct {
  const era_arm_velocity_t*
    arm_velocities;    //!< The velocities of the profile to be executed.
  const double*
    timestamps;        //!< The timestamps associated with the velocities [s].
  int num_velocities;  //!< The number of velocities in the profile.
} era_controller_argument_t;

/** \brief Static variable containing the controller thread */
extern era_thread_t era_controller_thread;

/** \brief Start the motion controller
  * Performs a timed feed of velocity commands to the motors.
  * \param[in] arm_velocities The arm velocities that will be fed to the
  *   motors.
  * \param[in] timestamps An array of absolute timestamps associated with
  *   the arm velocities [s].
  * \param[in] num_velocities The number of velocities contained in the
  *   profile.
  * \return The resulting error code.
  */
int era_controller_start(
  const era_arm_velocity_t* arm_velocities,
  const double* timestamps,
  int num_velocities);

/** \brief Exit the motion controller
  */
void era_controller_exit(void);

/** \brief Run the motion controller
  * This function is run within the controller thread and should never
  * be called directly.
  * \param[in] arg The arguments passed to the controller thread.
  * \return The result of the controller thread.
  */
void* era_controller_run(void* arg);

#endif
