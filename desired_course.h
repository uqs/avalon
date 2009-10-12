/************************************************************************/
/*								                                    	*/
/*		                 P R O J E K T    A V A L O N          			*/
/*								                                    	*/
/*	         desired_course.h		Stores the heading to next waypoint */
/*								                                    	*/
/*	            Last Change	15.April 2009; Hendrik Erckens          	*/
/*							                                    		*/
/************************************************************************/


#ifndef DESIRED_COURSE_H
#define DESIRED_COURSE_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(DesiredHeading,
		struct 
        {
            double heading; //in degrees, initialized with 400 so skipper knows when a programm gets called
        }
);
#endif //desired_course.h
