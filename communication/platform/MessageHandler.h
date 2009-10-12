#ifndef MESSAGEHANDLER_H_
#define MESSAGEHANDLER_H_

#include <vector>
#include <string>

/*** abstract class, basically a java interface
 * 
 */
class MessageHandler
{
// data types
public:
	typedef enum {
		FAIL,
		OK,
	} RequestStatus;
	
	typedef unsigned char byte;
// methods
public:
	virtual ~MessageHandler() {};
	virtual	RequestStatus send_status(const std::string &status) = 0;
	virtual RequestStatus receive_message(std::string &message) = 0;
};

#endif /*MESSAGEHANDLER_H_*/
