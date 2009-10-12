/************************************************************************/
/*									*/
/*		       P R O J E K T    C A S T O R 			*/
/*								 	*/
/*	windcleaner.h		The Wind-Clean-Class				*/
/*									*/
/*	Last Change	13.April 2009; Gion-Andri Büsser	*/
/*									*/
/************************************************************************/

#ifndef WINDCLEANER_H
#define WINDCLEANER_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(WindCleanData,
		struct {
			double speed; //in knoten, always real
			double speed_long; //in knoten, always real
			double angle_of_attack_app; //from -180° to 180°
			double bearing_app; //from -180° to 180°
			double bearing_real; //from -180° to 180°
			double global_direction_app; //from -180° to 180°
			double global_direction_real; //from -180° to 180°
			double global_direction_real_long; //from -180° to 180°

		}
);


#endif //windcleaner
