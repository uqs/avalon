/************************************************************************/
/*									                                    */
/*		       P R O J E K T    A V A L O N 			                */
/*								 	                                    */
/*	imucleaner.h		The Imu-Clean-Class				                */
/*									                                    */
/*	Last Change	May 4th 2009; Hendrik Erckens                           */
/*									                                    */
/************************************************************************/

#ifndef IMUCLEANER_H
#define IMUCLEANER_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(imuCleanData,
		struct
        {
			struct {double roll;} attitude; // in °

			struct {double x; double y; double z; double drift;} velocity; // velocity in knots
		}
);


#endif //imucleaner
