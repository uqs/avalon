#ifndef MESSAGEMANAGER_H_
#define MESSAGEMANAGER_H_

#include "MessageHandler.h"
#include "TransportHandler.h"
#include "SafeMemory.h"
#include "Thread.h"
#include "Log.h"
#include "PowerManager.h"
#include "SoftwareWatchdog.h"
#include "configlib/configfile.h"
#include "configlib/configitem.h"


#include <string>
#include <vector>
#include <pthread.h>
#include <semaphore.h>

class MessageManager : public MessageHandler, public Thread, public Log
{
// types
private:
	struct Event {
		enum Type {
			SEND_STATUS_MESSAGE, FETCH_COMMAND_MESSAGE
		};
		int time;
		Type type;
		int tries;
		Event(Type t, int moment) : time(moment), type(t), tries(0) {};
		bool operator<(const Event &other) const { return time > other.time; };
	};
	
	class EventQueue {
		private:
			/// storage space for the queue. whenever the mutex is unlocked,
			/// queue is garanteed to be of heap structure
			std::vector<Event> queue;
			pthread_mutex_t mutex;
	
		public:
			EventQueue();
			~EventQueue();
			void push_event(const Event &e);
			Event pop_event();
			bool has_pending();
			int next_timeout();
			int remove_pending(MessageManager::Event::Type t);
			int size() { return queue.size(); };
	};

// data members
private:
	sem_t main_semaphore;
	
	// thread safe memory
	SafeMemory<std::string> status_message;
	SafeMemory<std::string> command_message;
	
	std::vector<TransportHandler*> handler_list;
	TransportHandler *current_handler;
	
	EventQueue event_queue;
	SoftwareWatchdog watchdog;
	PowerManager pm;
	
	static const int STATUS_MESSAGE_RETRY_LIMIT;
	static const int STATUS_MESSAGE_RETRY_PERIOD;
	static const int COMMAND_MESSAGE_PERIOD;
	static const int FORECAST_FETCH_RETRY_LIMIT;
	static const int FORECAST_FETCH_RETRY_PERIOD;
	static const int MAIL_SEND_RETRY_LIMIT;
	static const int MAXIMAL_TIMEOUT;
	
	configlib::configitem<std::string> mail_server_outgoing;
	configlib::configitem<std::string> mail_server_incoming;
	configlib::configitem<std::string> mail_server_login;
	configlib::configitem<std::string> mail_server_password;
	configlib::configitem<std::string> mail_expeditor;
	configlib::configitem<std::string> mail_receipient;

// methods
private:
	TransportHandler *best_handler();
	void send_email(std::string message);
	std::vector<byte> receive_email();
	void handle_event(const Event& e);
	void handle_status_message(Event e);
	void handle_command_message(Event e);
	int get_first_timeout(std::string configuration);

public:
	MessageManager(configlib::configfile& config);
	virtual ~MessageManager();

	virtual int run();
	virtual	RequestStatus send_status(const std::string &status);
	virtual RequestStatus receive_message(std::string &message);
	void register_transport_handler(TransportHandler *handler);
};

#endif /*MESSAGEMANAGER_H_*/
