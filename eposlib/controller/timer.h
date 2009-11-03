/*	Header-file for
 *	BlueBotics ERA-5/1 controller timer
 *
 * 	Ralf Kaestner    ralf.kaestner@gmail.com
 * 	Last change:     9.5.2008
 */

#ifndef _TIMER_H
#define _TIMER_H

/** \file
  * \brief The BlueBotics ERA-5/1 controller timer
  *
  * The timer of the motion controller for the BlueBotics ERA-5/1.
  */

/** \brief Start the timer
  * \param[out] timestamp The timestamp that will contain the start time.
  */
void era_timer_start(
  double* timestamp);

/** \brief Stop the timer and return the elapsed time
  * \param[in] timestamp The timestamp containing the timer's start time.
  * \return The ellapsed time in [s].
  */
double era_timer_stop(
  double timestamp);

/** \brief Get the timer frequency
  * \param[in] timestamp The timestamp containing the timer's start time.
  * \return The timer frequency in [Hz].
  */
double era_timer_get_frequency(
  double timestamp);

/** \brief Wait for the expiration of the timer
  * \param[in] timestamp The timestamp containing the timer's start time.
  * \param[in] frequency The frequency that corresponds to the timer's
  *   expiration period.
  * \return The resulting error code. If the given frequency cannot be
  *   reached by the timer, an ERA_ERROR_TIMER_FAULT will be returned.
  */
int era_timer_wait(
  double timestamp,
  double frequency);

/** \brief Sleep for a specified amount of time
  * \param[in] seconds The sleep duration in [s].
  * \return The resulting error code.
  */
int era_timer_sleep(
  double seconds);

#endif
