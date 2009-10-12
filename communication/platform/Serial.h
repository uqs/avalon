//
// C++ Interface: Serial
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SERIAL_H
#define SERIAL_H

#include <string>
#include <vector>
#include <termios.h>

/**
 * Abstraction and encapsulation of a serial port intendet to communicate with
 * a modem. Supports timeouts for reading as well as writing by putting the
 * port in nonblocking mode and using select().
 *
 * Special functions facilitate reading data until certain keywords are found.
 *
 * An object of this class can be reused on another port by calling the open()
 * method with different arguments. The previously opened port will be closed
 * automatically.
 *
 * @author David Frey, ETZ G71.1 <freyd@ee.ethz.ch>
 */
class Serial {
	static const char lockfile_prefix[];

	int file_descriptor;
	bool was_blocking;
	struct termios oldtio;
	int read_timeout;
	int write_timeout;
	std::string lockfile;

public:
	Serial();
	Serial(std::string device, tcflag_t speed, bool rtscts);
	
	~Serial();
	void open(std::string device, tcflag_t speed, bool rtscts);
	void close();
	bool write(const std::string &data);
	std::string read();
	unsigned int wait_for(const std::vector<std::string> &words, std::string *read);
	void set_read_timeout(int timeout);
	void set_write_timeout(int timeout);

};

#endif
