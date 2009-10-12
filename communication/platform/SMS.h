//
// C++ Interface: SMS
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SMS_H
#define SMS_H

#include <string>

/**
 * Class to de- and encode the SMS PDU format.
 * 
 * To decode, use set_data() to set the encoded string and then call decode().
 * Afterwards you can read out the decoded parts using their get_ methods.
 *
 * To encode you set all required parts using the set_ methods and thenn call
 * encode(). The result is obtainable with get_data() and get_data_length().
 *
 * All code is non-validating, so expect exceptions to be thrown if you supply
 * invalid data.
 *
 * References: http://www.twit88.com/home/utility/sms-pdu-encode-decode
 * http://www.dreamfabric.com/sms/
 */
class SMS {
// methods
public:
	SMS();
	~SMS();
	void encode();
	void decode();
	void set_destination_number(std::string number);
	std::string get_origin_number();
	void set_text(std::string t);
	std::string get_text();
	void set_data(std::string d);
	std::string get_data();
	void set_service_center_number(std::string number);
	int get_data_length();

// data members
private:
	int data_length;
	std::string service_center_number;
	std::string destination_number;
	std::string origin_number;
	std::string text;
	std::string data;

	std::string encode_number(const std::string &number);
	std::string decode_number(const std::string &encoded, unsigned int length);
	std::string encode_string(const std::string &s);
	std::string decode_string(const std::string &s, unsigned int length);
	unsigned char hex2char(const std::string &s, int pos);
};

#endif
