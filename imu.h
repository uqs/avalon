/************************************************************************/
/*									                                    */
/*		               P R O J E K T    A V A L O N 			        */
/*								 	                                    */
/*	    imu.h		Sotre-Class for the IMU Data                        */
/*									                                    */
/*      Authors     Patrick Schwizer        patricsc@student.ethz.ch    */
/*                  Stefan Wismer           wismerst@student.ethz.ch    */
/*									                                    */
/************************************************************************/



#ifndef IMU_H
#define IMU_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(imuData,
		struct {
			double speed; //in KNOTS
			//GPS-Data
			struct {double longitude; double latitude; double altitude;} position;

			//IMU-Data

			struct {double roll; double pitch; double yaw;} attitude; // in deg

			struct {double x; double y; double z;} velocity; // velocity in x, y, z in knots

			struct {double x; double y; double z;} acceleration; // acceleration in x, y, z in m/s^2

			struct {double x; double y; double z;} gyro; // gyroscope data in x, y, z in deg/s
				
			double temperature; // in deg C				
		}
);


#endif //IMU_H
