/************************************************************************/
/*									                                    */
/*		               P R O J E K T    A V A L O N 			        */
/*								 	                                    */
/*	    gps.h		Store-Class for GPS-Data                            */
/*									                                    */
/*      Authors     Patrick Schwizer        patricsc@student.ethz.ch    */
/*                  Stefan Wismer           wismerst@student.ethz.ch    */
/*									                                    */
/************************************************************************/



#ifndef GPS_H
#define GPS_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(gpsData,
		struct {
			//GPS_UTC_TIME time;
			struct {double longitude; double latitude;double altitude;} position;

			short dgpsage;
			short dgpsref;
			float HDOP;
			short gpsFIX;
			int sat;
			double speed_kmph;
			double speed_kn;
			double course;
			
		}
);


#endif //GPS_H
