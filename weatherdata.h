/************************************************************************/
/*									                                    */
/*		                P R O J E K T    A V A L O N 			        */
/*								 	                                    */
/*	    weatherdata.h	Struct for the weather-data from satellite	    */
/*									                                    */
/*	    Author    	    Stefan Wismer               			        */
/*                      wismerst@student.ethz.ch                        */
/*									                                    */
/************************************************************************/

#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(Weather,
  struct {
	// TODO: A better solution has to be found for timestamps. int is too
    // small.
    int timestamp;		        // When was this data received?
	int forecast_timestamp;	    // What time is is valid?

    float xdata[161][81];		// Wind data
    float ydata[161][81];		// Wind data
  }
);

#endif // WEATHER_DATA_H
