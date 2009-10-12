//
// C++ Implementation: Serial
//
// Description: 
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Serial.h"
#include "Exception.h"

#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <fstream>
#include <iostream>

const char Serial::lockfile_prefix[] = "/var/lock/LCK..";


Serial::Serial() : file_descriptor(-1), read_timeout(10), write_timeout(10)
{
}

Serial::Serial(std::string device, tcflag_t speed, bool rtscts) :
		 file_descriptor(-1), read_timeout(10), write_timeout(10)
{
	open(device, speed, rtscts);
}


Serial::~Serial()
{
	close();
}


/**
 * Open and initialize a certain serial device. The current settings of the port
 * are stored to be reapplied when the device is closed. The port is put into
 * nonblocking mode, so that timeouts can be implemented using select().
 * If the port was already opened, it is closed and then reopened.
 *
 * @param device	name of the device file
 * @param speed		a speed flag as defined in bits/termios.h, e.g. B19200,
 *                      B38400, B115200, etc.
 * @param rtscts	if hardware flow control, i.e. RTS/CTS should be used
 */
void Serial::open(std::string device, tcflag_t speed, bool rtscts)
{
	if(file_descriptor != -1) {
		close();
	}

	// create a lock file, see http://www.pathname.com/fhs/pub/fhs-2.3.html#VARLOCKLOCKFILES
	lockfile = lockfile_prefix + device.substr(device.find_last_of('/'));
	// unfortunately we cannot do this with portable C++
	int lock_fd = ::open(lockfile.c_str(), O_EXCL | O_CREAT);
	if(lock_fd == -1) {
		if(errno != EEXIST) {
			throw(Exception("couldn't create lockfile " + lockfile, errno));
		}
		// lock file exists. check if it is stale
		std::ifstream lock_stream(lockfile.c_str());
		int pid = 0;
		lock_stream >> pid;
		if(kill(pid, 0) == 0) {
			throw(Exception("serial device is in use: " + device));
		}
		// it's a stale lockfile, remove it
		unlink(lockfile.c_str());
		int lock_fd = ::open(lockfile.c_str(), O_EXCL | O_CREAT);
		if(lock_fd == -1) {
			throw(Exception("couldn't create lockfile even after removing a stale one " + lockfile, errno));
		}
	}
	dprintf(lock_fd, "%10i", getpid());
	::close(lock_fd);
	
	// now open the device file
	// code from http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
	struct termios newtio;
	file_descriptor = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if(file_descriptor == -1) {
		throw(Exception("couldn't open serial device " + device, errno));
	}

	tcgetattr(file_descriptor, &oldtio); /* save current serial port settings */
	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

        /* 
	BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	CRTSCTS : output hardware flow control (only used if the cable has
	all necessary lines. See sect. 7 of Serial-HOWTO)
	CS8     : 8n1 (8bit,no parity,1 stopbit)
	CLOCAL  : local connection, no modem contol
	CREAD   : enable receiving characters
	*/
	newtio.c_cflag = speed |CS8 | CLOCAL | CREAD;
	if(rtscts) {
		newtio.c_cflag |= CRTSCTS;
	}

        /*
	IGNPAR  : ignore bytes with parity errors
	otherwise make device raw (no other input processing)
	*/
	newtio.c_iflag = IGNPAR;

        /*
	Raw output.
	*/
	newtio.c_oflag = 0;

	/*
	ICANON  : enable canonical input
	disable all echo functionality, and don't send signals to calling program
	*/
	newtio.c_lflag = 0;
	// these don't really have an effect, as the port is in nonblocking mode
	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	newtio.c_cc[VTIME]    = 0;	// wait forever
	cfsetispeed(&newtio, speed);
	cfsetospeed(&newtio, speed);
	/* 
	now clean the modem line and activate the settings for the port
	*/
	if(tcflush(file_descriptor, TCIFLUSH) == -1) {
		throw(Exception("couldn't fulsh serial device", errno));
		
	}
	if(tcsetattr(file_descriptor, TCSANOW, &newtio) == -1) {
		throw(Exception("couldn't change serial device settings", errno));
		
	}
	
	// make the port non blocking
	int flags = fcntl(file_descriptor, F_GETFL);
	if(flags & O_NONBLOCK) {
		was_blocking = false;
	}
	else {
		if(fcntl(file_descriptor, F_SETFL, flags | O_NONBLOCK) == -1) {
			throw(Exception("couldn't set serial device to non blocking", errno));
		}
		was_blocking = true;
	}

}


/**
 * Close the associated serial port and restore all settings to how they were
 * before.
 */
void Serial::close()
{
	if(file_descriptor != -1) {
		// restore old attributes
		tcsetattr(file_descriptor, TCSANOW, &oldtio);
		// restore blocking state
		if(was_blocking) {
			int flags = fcntl(file_descriptor, F_GETFL);
			if(fcntl(file_descriptor, F_SETFL, flags & ~O_NONBLOCK) == -1) {
				throw(Exception("couldn't set serial device to blocking", errno));
			
			}
		}
		::close(file_descriptor);
		unlink(lockfile.c_str());
		
		file_descriptor = -1;
	}
}


/**
 * Write a string of data to the serial device. Respects the timeout set by
 * set_write_timeout() accumulatively. This guarantees that a call to write()
 * will never last longer than write_timeout.
 *
 * @param data data to be written to the serial port
 */
bool Serial::write(const std::string &data)
{
	int written = 0;
	int n;
	struct timeval timeout;
	timeout.tv_sec = write_timeout;
	timeout.tv_usec = 0;
	fd_set write_fds;
	FD_ZERO(&write_fds);
	FD_SET(file_descriptor, &write_fds);
	while(written < (signed)data.length()) {
		n = select(file_descriptor+1, NULL, &write_fds, NULL, &timeout);
		if(n == 0) {	// timeout
			throw(TimeoutException("waiting to write to serial device"));
		}
		if(n == -1) {
			throw(Exception("waiting to write to serial device", errno));
		}
		if(n != 1) {
			throw(Exception("strange things while waiting to write to serial device"));
		}
		// now serial port is ready to be written to.
		n = ::write(file_descriptor, data.c_str() + written, data.length() - written);
		if(n == -1) {
			if(errno == EAGAIN) {	// spurious return of select. what the hell...
				continue;
			}
			throw(Exception("writing to serial device", errno));
		}
		written += n;
	}
	if(tcdrain(file_descriptor) == -1) {
		throw(Exception("writing to serial device", errno));
	}
	std::cerr << "wrote: " << data << std::endl;
	return true;
}


/**
 * Read the first chunk of data that is available.
 * Currently not used.
 */
std::string Serial::read()
{
	const int bufsize = 255;
	char buffer[bufsize];
	int n;
	std::string ret;
	struct timeval timeout;
	timeout.tv_sec = read_timeout;
	timeout.tv_usec = 0;
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(file_descriptor, &read_fds);
	do {
		n = select(file_descriptor+1, &read_fds, NULL, NULL, &timeout);
		if(n == 0) {	// timeout
			throw(TimeoutException("waiting to read from serial device"));
		}
		if(n == -1) {
			throw(Exception("waiting to read from serial device", errno));
		}
		if(n != 1) {
			throw(Exception("strange things while waiting to read from serial device"));
		}
		// now the serial port has some bytes for us
		n = ::read(file_descriptor, buffer, bufsize);
		if(n == -1) {
			if(errno == EAGAIN) {	// spurious return of select.
				continue;	// ignore
			}
			throw(Exception("reading from serial device", errno));
		}
		ret.insert(ret.end(), buffer, buffer + n);
		std::cerr << "read: " << ret << std::flush;
	} while(n == bufsize);
	return ret;
}


/**
 * Read from the serial port until one of the supplied words is found or
 * the current read timeout has expired. The timeout starts at the function call
 * and is not reset until returning. I.e. it is guaranteed that the functcion
 * doesn't do IO longer than allowed by the read timeout.
 *
 * @param words	a list of words to be searched for in the output
 * @param read	a pointer to a string variable where the received text should be stored.
 * @return	the index of the word that was found
 */
unsigned int Serial::wait_for(const std::vector<std::string> &words, std::string *read)
{
	const int bufsize = 50;
	char buffer[bufsize];
	bool done = false;
	int n;
	unsigned int i;
	std::string received;
	struct timeval timeout;
	timeout.tv_sec = read_timeout;
	timeout.tv_usec = 0;
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(file_descriptor, &read_fds);
	do {
		n = select(file_descriptor+1, &read_fds, NULL, NULL, &timeout);
		if(n == 0) {	// timeout
			throw(TimeoutException("waiting to read from serial device"));
		}
		if(n == -1) {
			throw(Exception("waiting to read from serial device", errno));
		}
		if(n != 1) {
			throw(Exception("strange things while waiting to read from serial device"));
		}
		// now the serial port has some bytes for us
		n = ::read(file_descriptor, buffer, bufsize);
		if(n == -1) {
			if(errno == EAGAIN) {	// spurious return of select. what the hell...
				continue;
			}
			throw(Exception("reading from serial device", errno));
		}
		received.insert(received.end(), buffer, buffer + n);
		for(i = 0; i < words.size(); i++) {
			if(received.find(words[i]) != std::string::npos) {
				done = true;
				break;
			}
		}
	} while(!done);
	std::cerr << "read: " << received << std::flush;
	std::cerr << "found option " << i << std::endl;
	if(read != NULL) {
		*read = received;
	}
	return i;
}


/**
 * Set the new read timeout in seconds. Default is 10 seconds.
 * @param timeout
 */
void Serial::set_read_timeout(int timeout)
{
	read_timeout = timeout;
}


/**
 * Set the new write timeout in seconds. Default is 10 seconds.
 * @param timeout
 */
void Serial::set_write_timeout(int timeout)
{
	write_timeout = timeout;
}
