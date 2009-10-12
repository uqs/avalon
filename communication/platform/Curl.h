//
// C++ Interface: Curl
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CURL_H
#define CURL_H

#include <curl/curl.h>
#include <vector>
#include <istream>


/**
 * C++ wrapper for the cURL library http://curl.haxx.se/libcurl/
 * Objects are not thread safe, don't use the same objet in more than one thread.
 */
class Curl
{
// data members
private:
	static bool initialized;
	static const long CONNECTION_TIMEOUT;
	
	CURL *handle;
	std::vector<char> get_buffer;
	std::vector<char>::const_iterator put_position;
	const std::vector<char> *put_buffer;
	std::istream *put_stream;
	char error_buffer[CURL_ERROR_SIZE];
	enum {
		PUT_VECTOR, PUT_STREAM
	} put_mode;

// methods
public:
	Curl();
	~Curl();
	std::vector<char> fetch(std::string url);
	void put(std::string url, const std::vector<char> &data);
	void put(std::string url, std::istream *data);
	std::string urlencode(const std::string &c);


private:
	static size_t curl_write_data(void *buffer, size_t size, size_t nmemb, void *userp);
	static size_t curl_read_data(char *bufptr, size_t size, size_t nitems, void *userp);
	size_t write_data(char *data, size_t size);
	size_t read_data(char *data, size_t size);
};

#endif
