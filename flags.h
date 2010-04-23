/************************************************************************/
/*																		*/
/*		       P R O J E K T    A V A L O N 							*/
/*								 										*/
/*	flags.h	    Defines System states				                	*/
/*																		*/
/*	Last Change	26. March 2009; Hendrik Erckens							*/
/*																		*/
/************************************************************************/

#ifndef FLAGS_H
#define FLAGS_H

#include <DDXStore.h>
#include <DDXVariable.h>

/**
  * Defines for all Navigator-Programs
 **/

#define AV_FLAGS_NAVI_IDLE              1
#define AV_FLAGS_NAVI_NEWCALCULATION    2
#define AV_FLAGS_NAVI_NORMALNAVIGATION  3
#define AV_FLAGS_NAVI_GOAL_APPROACH     8

#define AV_FLAGS_GLOBALSK_LOCATOR       21
#define AV_FLAGS_GLOBALSK_TRACKER       22
#define AV_FLAGS_GLOBALSK_CLOSING       23
#define AV_FLAGS_GLOBALSK_AVOIDANCE	24
#define AV_FLAGS_GLOBALSK_SURVIVE	25

/**
  * Defines for Sailor States
  **/
#define AV_FLAGS_ST_IDLE 1
#define AV_FLAGS_ST_DOCK 2
#define AV_FLAGS_ST_NORMALSAILING 3
#define AV_FLAGS_ST_TACK 4
#define AV_FLAGS_ST_JIBE 5
#define AV_FLAGS_ST_UPWINDSAILING 6
#define AV_FLAGS_ST_DOWNWINDSAILING 7
#define AV_FLAGS_ST_MAXENERGYSAVING 8
#define AV_FLAGS_ST_HEADINGCHANGE 9
#define AV_FLAGS_SAIL_DIR_NOPREFERENCE 0
#define AV_FLAGS_SAIL_DIR_ZERO 1
#define AV_FLAGS_SAIL_DIR_FRONT 2

/**
 * Defines for Man In Charge
 **/
#define AV_FLAGS_MIC_NOONE 0
#define AV_FLAGS_MIC_SAILOR 1
#define AV_FLAGS_MIC_REMOTECONTROL 2

/**
 * General Flags
 **/

DDX_STORE_TYPE(Flags,
    struct 
    {
        int man_in_charge;          // who is allowed to write angles?
                                    // 0 - emergeny stop (nobody writes - wait
                                    //     joystick to release the stop)
                                    // 1 - sailor
                                    // 2 - remotecontroller
    
        int state;                  // The current state of the sailor 
                                    // set by flags-checker.
                                    // Sailor statemachine depends on this 

        int state_requested;        // The state that other programs
                                    // requested. If request is
                                    // valid, sailor_transitions will
                                    // switch to this state

        int sail_direction;         // specifies in what direction the
                                    // sail should be turned during
                                    // HEADINGCHANGE

        int sailor_no_tack_or_jibe; // If true, sailor_statemachine will
                                    // stay at current heading


        int navi_state;             // shows the status of the navigator-program
        unsigned long navi_index_call;       // shows which calculation is currently beeing calculated!
        unsigned long navi_index_answer;

	unsigned int skip_index_dest_call;       // changes if the global skipper changes destination point and request a new calculation
//         unsigned long skip_index_dest_answer;

        int autonom_navigation;
        int global_locator;         // to decide which state the global skipper is in 

        
        int joystick_timeout;	    // true if joystick has timeout
    }   
);

/**
 * Individual Flags for the Programs
 **/ 

DDX_STORE_TYPE(rcFlags,
    struct
    {
        int emergency_stop;             // true if the joystick asked for an emergency Stop (set to wind)
        int motion_stop;                // true if the joystick asked for an emergency motion stop (boom)
        int joystick_timeout;		    // true if joystick has timeout
        int sailorstate_requested;      // remote control requests this sailor state
        int man_in_charge;              // requested man in charge
        int autonom_navigation;        // 0: nothing autonomous; 1: everything works autonomously
    }
);

DDX_STORE_TYPE(sailorFlags,
    struct
    {
        int state;                  // Sailor Transitions requests this state
                                    // and wants flags-checker to set it
        int no_tack_or_jibe;        // If true, sailor_statemachine will
                                    // stay at current heading
        int sail_direction;         // specifies in what direction the
                                    // sail should be turned during
                                    // HEADINGCHANGE
    }
);



DDX_STORE_TYPE(NaviFlags,
    struct
    {
        int navi_state; 
        
        unsigned long navi_index_call;       // shows which calculation is currently beeing calculated!
        unsigned long navi_index_answer;      
    }
);

DDX_STORE_TYPE(SkipperFlags,
        struct
        {
        int global_locator;     //see generalflags for explanation
	
	unsigned int skip_index_dest_call;       // changes if the global skipper changes destination point and request a new calculation
//         unsigned long skip_index_dest_answer;
        }
);

#endif // FLAGS_H
