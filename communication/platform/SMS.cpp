//
// C++ Implementation: SMS
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SMS.h"

#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// iridium service center: +881662900005
// swisscom service center: +41794999000

SMS::SMS()
{
}


SMS::~SMS()
{
}


/**
 * Encodes the given text, destination and service center number in a PDU string.
 * No validation is performed on the data, so expect exception being thrown on
 * invalid input data.
 */
void SMS::encode()
{
	std::ostringstream pdu;
	pdu << std::hex << std::uppercase << std::setfill('0');
	int address_length;
	data = "";
	
	// service center number
	std::string number = encode_number(service_center_number);
	pdu << std::setw(2) << (int)(number.length() / 2);
	pdu << number;
	
	address_length = pdu.str().length();
	
	pdu << "11";	// outgoing message with validity period field
	pdu << "00";	// TP-Message-Reference
	
	// destination number
	number = encode_number(destination_number);
	pdu << std::setw(2) << (destination_number.length()-1);
	pdu << number;
	
	pdu << "00";	// TP-PID. Protocol identifier: SMS
	pdu << "00";	// TP-DCS. Data coding scheme: 7-bit, uncompressed
	
	pdu << "A7";	// validity period: 1 day
	
	pdu << std::setw(2) << text.length();
	pdu << encode_string(text);
	
	data = pdu.str();
	data_length = (data.length() - address_length) / 2;
}

/**
 * Decode the PDU data that was supplied with set_data. Doesn't validate the
 * inputs, so expect Exceptions being thrown if data is invalid. Leaves the
 * output fields blank if no input was given.
 */
void SMS::decode()
{
	unsigned int pos = 0;
	unsigned int smsc_length, number_length;
	std::istringstream in;
	if(data.length() < 3) {
		return;
	}
	data_length = 0;
	destination_number = service_center_number = text = "";
	
	//read service center number
	in.str(data.substr(pos,2)); pos += 2;
	in >> std::hex >> smsc_length;	// length in octets
	service_center_number = decode_number(data.substr(pos, 2*smsc_length), 2*smsc_length-2);
	pos += smsc_length*2;
	
	pos += 2; // skip First octet of this SMS-DELIVER message.

	std::string aoi = data.substr(pos,2);
	in.str(aoi); in.clear(); pos += 2;
	in >> std::hex >> number_length;
	int number_width = number_length;
	if(number_width % 2) {
		number_width++;
	}
	origin_number = decode_number(data.substr(pos, 2 + number_width), number_length);
	pos += 2 + number_width;
	
	pos += 2; // skip protocol identifier
	pos += 2; // skip data encoding scheme
	pos += 14; // skip timestamp
	cout << "length at " << pos << endl;
	in.str(data.substr(pos,2)); in.clear(); pos += 2;
	in >> data_length;
	
	text = decode_string(data.substr(pos), data_length);
}


/**
 * Set the destination number of the text message.
 */
void SMS::set_destination_number(std::string number)
{
	destination_number = number;
}

/**
 * Read out the originating number of the text message
 */
std::string SMS::get_origin_number()
{
	return origin_number;
}

void SMS::set_text(std::string t)
{
	text = t;
}

std::string SMS::get_text()
{
	return text;
}

void SMS::set_data(std::string d)
{
	data = d;
}

std::string SMS::get_data()
{
	return data;
}

void SMS::set_service_center_number(std::string number)
{
	service_center_number = number;
}

int SMS::get_data_length() {
	return data_length;
}

/**
 * Encodes a phone number to PDU format.
 * @param number
 * @return	the encoded number, including the format prefix
 */
std::string SMS::encode_number(const std::string &number)
{
	std::ostringstream ret;
	std::string::const_iterator i = number.begin();
	if(*i == '+') {
		ret << "91";	// international format
		i++;
	}
	else {
		ret << "A1";	// national format
	}
	for(; i != number.end(); i++) {
		char c = *i;
		if(++i == number.end()) {
			ret << 'F' << c;
			break;
		}
		ret << *i << c;
	}
	return ret.str();
}

/**
 * decode a telephone number from PDU format
 *
 * @param encoded the encoded number including the type-of-address byte
 * @param length  the length of the decoded number in bytes, excluding '+'-prefix
 *
 * @return the decoded number as a string
 */
std::string SMS::decode_number(const std::string &encoded, unsigned int length)
{
	std::ostringstream number;
	if(encoded.substr(0,2) == "91") {
		number << '+';
	}
	for(unsigned int i = 0; i < length && i+3 < encoded.length(); i += 2) {
		number << encoded[i+3];
		if(i+1 < length) {
			number << encoded[i+2];
		}
	}
	return number.str();
}


/**
 * Encodes the given string in PDU 7-bit format.
 *
 * @param s	String to encode. Must only contain 7-bit characters, otherwise the result is undefined.
 * @return	the encoded string
 */
std::string SMS::encode_string(const std::string &s)
{
	unsigned int i, shift = 0;
	std::ostringstream ret;
	ret << std::hex << std::uppercase << std::setfill('0');
	for(i = 0; i + 1 < s.length(); i++) {
		ret << std::setw(2) << (int)(unsigned char)(s[i] >> shift | s[i+1] << (7 - shift));
		if(++shift == 7) {
			i++;
			shift = 0;
		}
	}
	if(i < s.length()) {
		ret << std::setw(2) << (int)(unsigned char)(s[i] >> shift);
	}
	
	return ret.str();
}

/**
 * Decodes the given string from PDU 7-bit format.
 *
 * @param s	String to decode.
 * @param length length of the result string
 * @return	the decoded string
 */
std::string SMS::decode_string(const std::string &s, unsigned int length)
{
	unsigned int i, shift = 0;
	std::ostringstream ret;
	for(i = 0; i + 3 < s.length() && ret.str().length() < length; i += 2) {
		ret << (unsigned char)( (hex2char(s, i) >> shift | hex2char(s, i+2) << (8-shift)) & 0x7f );
		if(shift-- == 0) {
			i -= 2;
			shift = 7;
		}
	}
	if(ret.str().length() < length && i+1 < s.length() && shift < 2) {
		ret << (char)( (hex2char(s, i) >> shift) & 0x7f );
	}
	
	return ret.str();
}

/**
 * Convert the hex-octet at position pos in string s to its character value.
 */
unsigned char SMS::hex2char(const std::string &s, int pos)
{
	char ret = 0, c;
	for(int i = 0; i < 2; i++) {
		c = s[pos++];
		ret <<= 4;
		if(c > '9') {
			ret += 10 + c - 'A';
		}
		else {
			ret += c - '0';
		}
	}
	return ret;
}
