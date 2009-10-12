#ifndef TRANSPORTHANDLER_H_
#define TRANSPORTHANDLER_H_

#include <string>

class TransportHandler
{
private:
	const int cost;
	// Interface to a networking technology
	
public:
	
	TransportHandler(int c);
	virtual ~TransportHandler();
	virtual bool is_available() = 0;
	virtual void open_ip_connection() = 0;
	virtual bool is_connected() = 0;
	virtual void close_ip_connection() throw() = 0;
	virtual bool send_short_message(std::string message);
	virtual std::string poll_short_message() = 0;
	virtual std::string get_name() const = 0;

	static bool compare(const TransportHandler *left, const TransportHandler *right)
	{
		return left->cost < right->cost;
	}

};

#endif /*TRANSPORTHANDLER_H_*/
