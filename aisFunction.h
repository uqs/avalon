/************************************************************************/
/*									*/
/*		       P R O J E K T    C A S T O R 			*/
/*								 	*/
/*	ais.h		die Windsensor-Klasse				*/
/*									*/
/*	Last Change	26.Februar 2009; Patrick Schwizer		*/
/*									*/
/************************************************************************/

#ifndef AISFUNCTION_H
#define AISFUNCTION_H

#include <DDXStore.h>
#include <DDXVariable.h>


void decode(RtxNMEA * msg, std::vector<unsigned char> & numeric);

unsigned long getBit(const std::vector<unsigned char> & numeric);

unsigned long getBitSequence(const std::vector<unsigned char> & numeric, 
	unsigned int start, unsigned int length);

std::string getBitString(const std::vector<unsigned char> & numeric, 
	unsigned int start, unsigned int length);

int rtx_nmea_read_ais( int port, RtxNMEA *mesg );

static int rtx_nmea_parse_ais(RtxNMEA *msg);



#endif //AISFUNCTION_H
