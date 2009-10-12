#ifndef ETHERNETHANDLER_H_
#define ETHERNETHANDLER_H_

#include "TransportHandler.h"

class EthernetHandler : public TransportHandler
{
public:
	EthernetHandler();
	virtual ~EthernetHandler();
	virtual bool is_available();
	virtual void open_ip_connection();
	virtual bool is_connected();
	virtual void close_ip_connection() throw();
	virtual std::string get_name() const { return "EthernetHandler"; };
	virtual std::string poll_short_message() { return std::string(); };
};

#endif /*ETHERNETHANDLER_H_*/
