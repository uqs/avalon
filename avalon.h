/************************************************************************/
/*																		*/
/*      		       P R O J E K T    A V A L O N						*/
/*																		*/
/*	    avalon.h	    ALL GENERAL CONSTANTS.					 		*/
/*																		*/
/*	    Autohrs         Gion-Andri Büsser   gbuesser@student.ethz.ch    */
/*                      Hendrik Erckens     herckens@student.ethz.ch    */
/*                      Stefan Wismer       wismerst@student.ethz.ch    */
/*																	  	*/
/************************************************************************/


#ifndef AVALON_H
#define AVALON_H
/////////////////////////////
// General Constants
/////////////////////////////

#define AV_EARTHRADIUS          6371000.785         //in meters
#define AV_PI 					3.141592654

/////////////////////////////
// System Constants
/////////////////////////////

#define AV_NUMBER_OF_SERIAL_PORTS   10              // How many /dev/ttyUSB.. do we have?

/////////////////////////////
// Settings for WindCleaner
/////////////////////////////

#define AV_CLEAN_LENGTH				5 		// number of last windData to be considered int between 3 and 99
#define AV_CLEAN_LENGTH_LONG	    90 		// number of last windData to be considered for navigator input

/////////////////////////////
// Settings for input devices
/////////////////////////////
#define AV_JOYSTICK_RES_X		        32767.0	        // Resolution (Ticks) of the Joystick axes
#define AV_JOYSTICK_RES_Z		        32767.0
#define AV_JOYSTICK_RUDDER_FINE		    20	            // The max. range of the fine-tuning (no shoot-button)
#define AV_JOYSTICK_RUDDER_SENSITIVITY	0.0001	        // Rudder sensitivity in the calibration mode
#define AV_SAILSTICK_Y_SENSITIVITY	    0.0001	        // Sensitivity if mode is 2
#define AV_SAILSTICK_4_5_SENSITIVITY	(-0.16)	        // Sensitivity if mode is 3
#define AV_DESHEADSTICK_4_5_SENSITIVITY (0.5)           // Sensitivity if controlling the desired_heading in sailing mode


////////////////////////////////////////////////
// Different modes to steer the sail by Joystick
////////////////////////////////////////////////
#define AV_SAILSTICK_THROTTLE		    1 	            // Trottle (z-axis): up = +max, center = 0°, down = -max
#define AV_SAILSTICK_Y_INTEGRATED	    2	            // Move fore and back: values are integrated.
#define AV_SAILSTICK_BUT_4_5		    3	            // Use Buttons 4 and 5


/////////////////////////////
// Settings for the Actuators
/////////////////////////////

// Rudder
#define AV_MAX_RUDDER_ANGLE			    30.0
#define AV_RUDDER_TICKS_PER_DEGREE	    (-1583)	        // Convert Ticks to degrees (rudder)
#define AV_NOF_RUDDER_EPOS			    2	            // How many Epos on the Rudders?
#define AV_MAX_RUDDER_CURRENT		    10000	        // mA max per Rudder EPOS
#define AV_MAX_RUDDER_CONT_CURRENT	    5000	        // mA max continous per Rudder EPOS
#define AV_RUDDER_SPEED				    9000	        // RPM
#define AV_RUDDER_ACCELERATION		    30000	        // RPM/s
#define AV_RUDDER_DECELERATION		    16000	        // RPM/s
#define AV_RUDDER_MAX_FOLLOWING_ERROR	100000001       // Ticks
#define AV_RUDDER_LEFT				    1
#define AV_RUDDER_RIGHT				    2

// Rudder Homing Stuff
#define AV_HOMING_MODE                  -1
#define AV_HOMING_SWITCH_SPEED          300
#define AV_HOMING_THRESHOLD             2000
#define AV_HOMING_ZERO_SPEED            500
#define AV_HOMING_POSITION              0
#define AV_HOMING_OFFSET                30000

// Sail
#define AV_MAX_SAIL_ANGLE			    180.0
#define AV_MAX_SAIL_CURRENT			    10000	        // mA max per Rudder EPOS
#define AV_MAX_SAIL_CONT_CURRENT	    5000	        // mA max continous per Rudder EPOS
#define AV_SAIL_SPEED				    9000	        // RPM
#define AV_SAIL_ACCELERATION		    5000	        // RPM/s
#define AV_SAIL_DECELERATION		    5000	        // RPM/s
#define AV_SAIL_MAX_FOLLOWING_ERROR	    100000000       // Ticks
#define AV_SAIL_TICKS_PER_DEGREE	    (-2*1711)	    // Convert Ticks to degrees (rudder)
#define AV_SAIL_BRAKE_MASK              0xFFFF          // Settings for the DigiOut ports
#define AV_SAIL_BRAKE_POLARITY          0xFFFF
#define AV_SAIL_BRAKE_OPEN              0x0000          // Setting for the brake to be open
#define AV_SAIL_BRAKE_CLOSE             0x000C          // Setting for the brake to be closed
#define AV_SAIL_NODE_ID                 0x0002          // Node ID of the Sail EPOS
#define AV_POTI_NODE_ID                 0x0008          // Node ID of the poti
#define AV_POTI_RESOLUTION		4096		// ticks per revolution (360°)

// If you want to have a fancy control room environnment, activate this!!
//#define AV_SAILMAIN_CONTROLCENTER

#define AV_SAILMAIN_KEEPALIVE_LOGFILE                   // TODO HE: Should only by defined if filesystem is RW!!!!!!!!

//////////////////////
// Settings for Sailor
//////////////////////

#define AV_SAIL_HYSTERESIS              0               // Obsolete... Sail is not moved if change is smaller than this [deg]
#define AV_SAILOR_DECREASE_RUDDER_THRESHOLD     2.0     // kn, start decreasing rudder angle if speed is higher than this
#define AV_SAILOR_WANTED_AOA            30.0            // deg, Wanted Angle Of Attack on the sail
#define AV_SAILOR_MAX_HEIGHT_TO_WIND    45.0            // deg, Minimum angle (bearing) to wind that is sailable in UPWINDSAILING
#define AV_SAILOR_MAX_DOWNWIND_ANGLE    160.0           // deg, Max Angle from the wind that is sailable in DOWNWINDSAILING
#define AV_SAILOR_UPWIND_MIN_SAIL_DEGREES   10.0            // deg, fixed position of sail in UPWINDSAILING
#define AV_SAILOR_DOWNWIND_SAIL_DEGREES 120.0           // deg, fixed position of sail in DOWNWINDSAILING
#define AV_SAILOR_EPSILON_TACK          5.0            // deg, Trans to NORMALSAILING if closer to desired_heading than this
#define AV_SAILOR_EPSILON_JIBE          5.0            // deg, Trans to NORMALSAILING if closer to desired_heading than this
#define AV_SAILOR_EPSILON_HEADINGCHANGE 15.0            // deg, Trans to NORMALSAILING if closer to wanted_sail_angle than this
#define AV_SAILOR_TACK_HYSTERESIS       30.0            // deg, angle that desired_heading has to be (used to be 20.0)
                                                        // "on the other side of the wind" before TACK is initiated
#define AV_SAILOR_JIBE_HYSTERESIS       19.0            // deg ,analog to TACK
#define AV_SAILOR_TACK_END_TIMEOUT      30.0            // s, after this time TACK is aborted
#define AV_SAILOR_JIBE_END_TIMEOUT      40.0            // s, after this time JIBE is aborted
#define AV_SAILOR_HEADINGCHANGE_END_TIMEOUT      40.0   // s, after this time HEADINGCHANGE is aborted
#define AV_SAILOR_HEADINGCHANGE_ERROR   180.0            // if heading error is bigger than this, trans to HEADINGCHANGE

/////////////////////////////
// Settings for the Navigators
/////////////////////////////

#define AV_NEIGHBORHOOD                    8              // define the grid range: OPTIONS: _8,_24
// #define GNUPLOT_ENABLED				               //enable the gnuplot-drawing
#define AV_NAVI_GRID_SIZE	           100            //distance in meters between two nodes!

#define AV_NAUT_MILE                    1852           //1 nautical mile in meters!!
////
#define AV_NAVI_LAKE_TOLERANCE_SOLLTRAJECTORY 20             //in meters
#define AV_NAVI_SEA_TOLERANCE_SOLLTRAJECTORY  100           //in meters
#define AV_NAVI_LAKE_DIST_FOR_MANEUVER        3             //in meters, alter after testing!!!

#define AV_NAVI_RADIUS_FOR_BUOY_MANEUVER      15             //in meters
#define AV_NAVI_BUOY_MAN_HEADINGDEV           AV_PI/4       //in radians!
#define AV_LAKE_TUNNEL                      60             //meters, to both sides


#endif // AVALON_H
