/************************************************************************/
/*									*/
/*		       P R O J E K T    A V A L O N 			*/
/*								 	*/
/*	ais.cpp		Producer for the ais-receiver. Captures all	*/
/*			Data and brings it to the Store			*/
/*									*/
/*	Last Change	March 2., 2009; Patrick Schwizer		*/
/*									*/
/************************************************************************/


#include <DDXStore.h>
#include <DDXVariable.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

#include <rtx/getopt.h>
#include <rtx/main.h>
#include <rtx/error.h>
#include <rtx/timer.h>
#include <rtx/thread.h>
#include <rtx/message.h>
#include <DDXSerialComm.h>
#include <rtx/nmea.h>

#include <algorithm>

#include "ais.h"
#include "aisFunction.h"


enum states { STATE_LOOK_FOR_START, STATE_LOOK_FOR_END, STATE_LOOK_FOR_CHECKSUM };

/**
 * Global variable for all DDX object
 * */
DDXStore store;
DDXVariable aisData;
DDXVariable aisStruct;


// Storage for the command line arguments
const char * varname_aisData = "aisData";
const char * varname_aisStruct = "aisStruct";
const char * commport = "/dev/ttyUSB0";
const char * producerHelpStr = "All Data coming from the AIS";

// Command line arguments processing
RtxGetopt producerOpts[] = {
    {"name", "name of the variable to register",
        {
            {RTX_GETOPT_STR, &varname_aisData, ""},
            RTX_GETOPT_END_ARG
        }
    },
    {"port", "name of the comm port to use",
        {
            {RTX_GETOPT_STR, &commport, ""},
            RTX_GETOPT_END_ARG
        }
    },
    RTX_GETOPT_END
};

void * ais_production_thread(void * dummy)
{ 

    AisData ais;				//gps-struct
    RtxTime time_curr;
    RtxTime time_curr2;
    double time_curr_d;
    double time_curr_d2;
    int fd; 				//handle to the serial port
    RtxNMEA mesg;				//
    char fieldbuffer[82];			//	
    std::vector<unsigned char> numeric;
    unsigned long message_type;
    unsigned long mmsi_buffer;
    string str;
    int i,p;
    int ship_index;


    //initializing
    aisData.t_readto(ais,0,0);
    ais.number_of_ships = 0;

    fd = rtx_serial_open(
            commport,		//char *dev,
            38400, 			//int  	baud
            8,			//int  	databits,
            1,			//int  	stopbits,
            RTX_SERIAL_PARITY_NONE,	//RtxSerialParity  	parity,
            RTX_SERIAL_FLOW_NONE,	//RtxSerialFlow  	flow,
            RTX_SERIAL_MODEM_OFF,	//RtxSerialModem  	modem,
            0,			//int  	vmin,
            0			//int  	vtime	 
            );	


    if (fd==-1){rtx_message("couldn't open ais-device");return 0;};

    float ten=10,sixtythousand=60000;
    while (1) {
        aisData.t_readto(ais,0,0);
        
        rtx_nmea_read_ais(fd, &mesg);
        rtx_nmea_extract(&mesg,0,fieldbuffer);

        char data[82];
        rtx_nmea_extract(&mesg,5,data);

        if(!strcasecmp(fieldbuffer,"AIVDM"))
        {

            // if a ship information is too old:
            p = 0;
            while (p < ais.number_of_ships)
            {
                rtx_time_get(&time_curr2); 
                time_curr_d2 = rtx_time_to_double(&time_curr2);
                if ((time_curr_d2 - time_curr_d) > 1200)
                {
                    ais.Ship[p].mmsi = ais.Ship[ais.number_of_ships-1].mmsi;
                    ais.Ship[p].navigational_status = ais.Ship[ais.number_of_ships-1].navigational_status;
                    ais.Ship[p].rate_of_turn = ais.Ship[ais.number_of_ships-1].rate_of_turn;
                    ais.Ship[p].speed_over_ground = ais.Ship[ais.number_of_ships-1].speed_over_ground;
                    ais.Ship[p].position_accuracy = ais.Ship[ais.number_of_ships-1].position_accuracy;
                    ais.Ship[p].longitude = ais.Ship[ais.number_of_ships-1].longitude;
                    ais.Ship[p].latitude = ais.Ship[ais.number_of_ships-1].latitude;
                    ais.Ship[p].course_over_ground = ais.Ship[ais.number_of_ships-1].course_over_ground;
                    ais.Ship[p].heading = ais.Ship[ais.number_of_ships-1].heading;
                    strcpy (ais.Ship[p].destination, ais.Ship[ais.number_of_ships - 1].destination);
                    ais.Ship[p].time_of_arrival = ais.Ship[ais.number_of_ships-1].time_of_arrival;
                    ais.Ship[p].timestamp = ais.Ship[ais.number_of_ships-1].timestamp;

                    ais.number_of_ships --;
                }
                p++;
            }


            decode(&mesg,numeric); //writes encoded-data to a 8-bit vector "numeric"

            message_type=getBitSequence(numeric,1,6);

            mmsi_buffer=getBitSequence(numeric,9,30);
            rtx_message("mmsi %d",mmsi_buffer);

            i = 0;
            ship_index = -1;

            // see if that ship is already stored
            while((i <  ais.number_of_ships))
            {
                if (ais.Ship[i].mmsi==mmsi_buffer)
                {
                    ship_index = i;
                    break;
                }
                else 
                {   
                    i++;
                }

            }
            // if a new ship is in sight
            if (ship_index == -1)
            {
                ship_index = ais.number_of_ships;
                ais.number_of_ships ++;
            }

            switch (message_type)
            {
                case 1:

                    break;   //TODO does this come here?

                case 2:

                    break;

                case 3:
                    ais.Ship[ship_index].navigational_status=getBitSequence(numeric,39,2);
                    ais.Ship[ship_index].rate_of_turn=getBitSequence(numeric,41,8);
                    ais.Ship[ship_index].speed_over_ground=getBitSequence(numeric,51,10) /ten;
                    ais.Ship[ship_index].position_accuracy=getBitSequence(numeric,61,1);
                    ais.Ship[ship_index].longitude=getBitSequence(numeric,62,28) / sixtythousand;
                    ais.Ship[ship_index].latitude=getBitSequence(numeric,90,27) / sixtythousand;
                    ais.Ship[ship_index].course_over_ground=getBitSequence(numeric,117,12) / ten;
                    ais.Ship[ship_index].heading=getBitSequence(numeric,129,9);

                    rtx_time_get (&time_curr); 
                    time_curr_d = rtx_time_to_double(&time_curr);
                    ais.Ship[ship_index].timestamp = time_curr_d;

                    aisData.t_writefrom(ais);

                    rtx_message("mmsi: %d, latitude: %d, longitude: %d, speed: %d, course: %d, i==%u",ais.Ship[i].mmsi,ais.Ship[i].latitude,ais.Ship[i].longitude,ais.Ship[i].speed_over_ground,ais.Ship[i].course_over_ground,i);
                    break;



                case 4:
                    ais.Ship[ship_index].position_accuracy=getBitSequence(numeric,79,1);
                    ais.Ship[ship_index].longitude=getBitSequence(numeric,80,28);
                    ais.Ship[ship_index].latitude=getBitSequence(numeric,108,27);

                    rtx_time_get (&time_curr); 
                    time_curr_d = rtx_time_to_double(&time_curr);
                    ais.Ship[ship_index].timestamp = time_curr_d;
                    
                    aisData.t_writefrom(ais);
                    break;

                case 5:					
                    ais.Ship[ship_index].time_of_arrival=getBitSequence(numeric,275,20);
                    str=getBitString(numeric,303,120);
                    strcpy (ais.Ship[ship_index].destination, str.c_str());

                    aisData.t_writefrom(ais);
                    break;

                default: 
                    rtx_message("decoding message %d failed", message_type);
                    break;	
            }
        };



    };
    rtx_serial_close(fd);
    return NULL;

}




// Error handling for C functions (return 0 on success)
#define DOC(c) {int ret = c;if (ret != 0) {rtx_error("Command "#c" failed with value %d",ret);return -1;}} 

// Error handling for C++ function (return true on success)
#define DOB(c) if (!(c)) {rtx_error("Command "#c" failed");return -1;} 

// Error handling for pointer-returning function (return NULL on failure)
#define DOP(c) if ((c)==NULL) {rtx_error("Command "#c" failed");return -1;} 


int main (int argc, char * argv[])
{

	RtxThread * th;
	int ret;

	// Process the command line
	if ((ret = RTX_GETOPT_CMD (producerOpts, argc, argv, NULL, 
					producerHelpStr)) == -1) {
		RTX_GETOPT_PRINT (producerOpts, argv[0], NULL, producerHelpStr);
		exit (1);
	}
	rtx_main_init ("store-producer", RTX_ERROR_STDERR);

	// Open the store
	DOB(store.open());

	// Register the DataExample data type
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisStruct));
	DOC(DDX_STORE_REGISTER_TYPE (store.getId(), AisData));

	// Create output variable
	DOB(store.registerVariable(aisStruct, varname_aisStruct, "AisStruct"));
	DOB(store.registerVariable(aisData, varname_aisData, "AisData"));






	// Start the working thread
	DOP(th = rtx_thread_create ("AIS thread", 0,
				RTX_THREAD_SCHED_OTHER, RTX_THREAD_PRIO_MIN, 0, 
				RTX_THREAD_CANCEL_DEFERRED,
				ais_production_thread, NULL,
				NULL, NULL));

	// Wait for Ctrl-C
	DOC (rtx_main_wait_shutdown (0));
	rtx_message_routine ("Caught SIGINT/SIGQUIT, shutting down ais driver ...");

	// Terminating the thread
	rtx_thread_destroy_sync (th);

	// The destructors will take care of cleaning up the memory
	return (0);
}

void decode(RtxNMEA * msg, std::vector<unsigned char> & numeric)
{
	unsigned char buffer[82];
	unsigned char *it = buffer;
	unsigned int i;
	std::vector<unsigned char> sixbits;
	rtx_nmea_extract(msg,5,(char*)buffer);
	while (*it) {
		sixbits.push_back(*it-48);
		it ++;
	}
	// Check that we have a multiple of 4
	while (sixbits.size() % 4) {
		sixbits.push_back(0);
	}

	numeric.clear();
	for (i=0;i<sixbits.size();i+=4) {
		unsigned char triplet[3];
		triplet[0] = (sixbits[i+0] << 2) | ((sixbits[i+1] >> 4) & 0x03);
		triplet[1] = (sixbits[i+1] << 4) | ((sixbits[i+2] >> 2) & 0x0F);
		triplet[3] = (sixbits[i+2] << 6) | ((sixbits[i+3] >> 0) & 0x3F);
		numeric.push_back(triplet[0]);
		numeric.push_back(triplet[1]);
		numeric.push_back(triplet[2]);
	}
}

unsigned long getBit(const std::vector<unsigned char> & numeric,
	unsigned int position)
{
	unsigned char bitMask[8] = {
		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
	}; 
	unsigned char bitShift[8] = {
		7, 6, 5, 4, 3, 2, 1, 0
	}; 
	position -= 1;
	unsigned char byte = numeric[position/8];
	unsigned char bit = position % 8;
	return (byte & bitMask[bit]) >> bitShift[bit];
	
}

unsigned long getBitSequence(const std::vector<unsigned char> & numeric,
	unsigned int start, unsigned int length)
{
	unsigned int i;
	unsigned long result=0;
	for (i=start;i<start+length;i++) {
		result = (result << 1) | getBit(numeric,i);
	}
	return result;					
}

std::string getBitString(const std::vector<unsigned char> & numeric,
	unsigned int start,unsigned int length)
{
	unsigned int i=start;
	unsigned int eight=8;
	std::string result="";
	while (i < start + length) {
		unsigned int charlen = min(eight,length-i);
		unsigned long letter = getBitSequence(numeric,i,
				charlen);
		result += (char)letter;
		i += charlen;
	}
	return result;
}

		

int rtx_nmea_read_ais( int port, RtxNMEA *mesg )
{
  int mesgLoop = 1;
  int checksum;
  int currLength = 0, checksumCalc = 0;
  char charBuff, currChecksum[2];
  enum states currState;

  currState = STATE_LOOK_FOR_START;     /* define starting state */

  /* read full NMEA message from port */
  while( mesgLoop == 1 )
  {
      if( rtx_serial_read_timeout(port,&charBuff,1,2.0) < 0 )
          return( - 1 );

      switch( currState )
      {
          case STATE_LOOK_FOR_START:        /** look for start of NMEA message **/
              if( charBuff == '!' )           /* check character for start '$' */
              {
                  currState = STATE_LOOK_FOR_END;
                  currLength = 0;
                  checksumCalc = 0;
              }
              break;
          case STATE_LOOK_FOR_END:          /** while reading in message for for end **/
              if( charBuff == '*' )       /* check for start of checksum character */
                  currState = STATE_LOOK_FOR_CHECKSUM;
              else
              {
                  mesg->nmeaMesg[currLength] = charBuff; /* add character to buffer */
                  checksumCalc = mesg->nmeaMesg[currLength]; /* calc checksum */  //vor dem = ein ^gelöscht
                  currLength++;
              }
              break;
          case STATE_LOOK_FOR_CHECKSUM:

              currChecksum[0] = charBuff;

              if( rtx_serial_read_timeout(port,&charBuff,1,2.0) < 0 )
                  return( - 1 );

              currChecksum[1] = charBuff;

              currChecksum[2] = '\0';
              mesgLoop = 0;    /* exit messaging loop */
              break;
      }
  }

  /* check message checksum */
  mesg->nmeaMesg[currLength] = '\0';
  sscanf (currChecksum, "%x", &checksum); /* convert message checksum to int */

  /** compare calc checksum with mesg checksum **/
  if( checksum != checksumCalc )
    return( -2 );
  return rtx_nmea_parse_ais(mesg);

}

static int rtx_nmea_parse_ais(RtxNMEA *msg) 
{
	char * p = msg->nmeaMesg;
	unsigned int i;
	msg->numFields = 0;
	for (i=0;i<82;i++)
		msg->nmeaField[i] = NULL;
	
	msg->nmeaField[msg->numFields] = p;
	msg->numFields += 1;
	while (*p && !isspace(*p)) {
		if ((p - msg->nmeaMesg) > 82) return -1;
		if ((*p == ',')||(*p == '*')) {
			msg->nmeaField[msg->numFields] = p+1;
			msg->numFields += 1;
			*p = 0;
		}
		p++;
	}
	msg->numFields += 1;
	*p = 0;
	return 0;
}
