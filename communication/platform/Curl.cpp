//
// C++ Implementation: Curl
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Curl.h"
#include "Exception.h"

bool Curl::initialized = false;
const long Curl::CONNECTION_TIMEOUT = 120;

Curl::Curl()
{
	if(!initialized) {
		if(curl_global_init(CURL_GLOBAL_ALL) != 0) {
			throw(Exception("initializing curl library"));
		}
		initialized = true;
	}
	
	handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, CONNECTION_TIMEOUT);
	
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, Curl::curl_write_data);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(handle, CURLOPT_READFUNCTION, Curl::curl_read_data);
	curl_easy_setopt(handle, CURLOPT_READDATA, this);
}


Curl::~Curl()
{
	curl_easy_cleanup(handle);
}


size_t Curl::write_data(char *data, size_t size)
{
	get_buffer.reserve(get_buffer.size() + size);
	get_buffer.insert(get_buffer.end(), data, data + size);
	return size;
}

size_t Curl::read_data(char *data, size_t size)
{
	unsigned int i;
	switch(put_mode) {
		case PUT_VECTOR:
			for(i = 0; i < size && put_position != put_buffer->end(); i++) {
				data[i] = *(put_position++);
			}
			return i;
		case PUT_STREAM:
			put_stream->read(data, size);
			return put_stream->gcount();
			break;
	}
	return 0;
}


size_t Curl::curl_write_data(void *buffer, size_t size, size_t nitems, void *userp)
{
	return static_cast<Curl*>(userp)->write_data((char*)buffer, size * nitems);
}

size_t Curl::curl_read_data(char *buffer, size_t size, size_t nitems, void *userp)
{
	return static_cast<Curl*>(userp)->read_data((char*)buffer, size * nitems);
}


/**
 * Fetches the content of an URL into a buffer.
 *
 * @param url	URL to be fetched, including the protocol specifier
 * @return	the contents of the URL
 */
std::vector<char> Curl::fetch(std::string url)
{
	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_UPLOAD, 0L);

	get_buffer.clear();
	if(curl_easy_perform(handle) != 0) {
		throw(Exception("fetching url " + url + ": " + error_buffer));
	}
	
	return get_buffer;
}

/**
 * Upload data to an URL. If the HTTP protocol is specified, uses the PUT method.
 *
 * @param url	URL to push data to
 * @param data
 */
void Curl::put(std::string url, const std::vector<char> &data)
{
	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, data.size());

	put_buffer = &data;
	put_position = data.begin();
	put_mode = PUT_VECTOR;
	
	if(curl_easy_perform(handle) != 0) {
		throw(Exception("fetching url " + url + ": " + error_buffer));
	}
}

/**
 * Upload data from a stream to an URL. If the HTTP protocol is specified,
 * uses the PUT method.
 *
 * @param url	URL to push data to
 * @param data	stream to read the data from
 */
void Curl::put(std::string url, std::istream *data) {
	// find out the size of the data
	std::streampos start = data->tellg();
	data->seekg(0, std::ios::end);
	int size = data->tellg() - start;
	data->seekg(start);

	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, size);

	put_stream = data;
	put_mode = PUT_STREAM;
}

/**
 * Encode data to be used in an URL, e.g for a HTTP GET request.
 *
 * @param c	data to be encoded
 * @return	encoded data
 */
std::string Curl::urlencode(const std::string &c)
{
	char *ret = curl_easy_escape(handle, c.c_str(), c.length());
	std::string escaped = ret;
	curl_free(ret);
	return escaped;
}
