#include "MessageManager.h"
#include "Exception.h"

#include <signal.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <algorithm>


const int MessageManager::STATUS_MESSAGE_RETRY_LIMIT = 3;
const int MessageManager::STATUS_MESSAGE_RETRY_PERIOD = 180;
const int MessageManager::COMMAND_MESSAGE_PERIOD = 3600;
const int MessageManager::FORECAST_FETCH_RETRY_LIMIT = 2;
const int MessageManager::FORECAST_FETCH_RETRY_PERIOD = 300;
const int MessageManager::MAIL_SEND_RETRY_LIMIT = 2;
const int MessageManager::MAXIMAL_TIMEOUT = 5*60-10;

MessageManager::MessageManager(configlib::configfile& config) :
		Log("MessageManager"), current_handler(NULL),
		watchdog(SoftwareWatchdog::ID_MESSAGEMANAGER, false),
		pm(false),
		mail_server_outgoing(config, "MessageManager", "outgoing mailserver", "", "smtp://smtp.googlemail.com"),
		mail_server_incoming(config, "MessageManager", "incoming mailserver", "", "imaps://imap.googlemail.com"),
		mail_server_login(config, "MessageManager", "mailserver login", "", "avalontheboat@googlemail.com"),
		mail_server_password(config, "MessageManager", "mailserver password", "", "thenewcastor"),
		mail_expeditor(config, "MessageManager", "mail expeditor", "", "avalontheboat@googlemail.com"),
		mail_receipient(config, "MessageManager", "mail recipient", "", "honggoff@gmx.ch")
{
	// initialise an inter-thread semaphore with value 0
	if(sem_init(&main_semaphore, 0, 0) == -1) {
		throw(Exception("initialising MessageManager semaphore", errno));
	}
}

MessageManager::~MessageManager()
{
}

int MessageManager::run()
{
	// block all signals
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);
	pthread_sigmask(SIG_SETMASK, &mask, NULL);
	struct timespec timeout;
	timeout.tv_nsec = 0;

	// add initial fetch task
	event_queue.push_event(Event(Event::FETCH_COMMAND_MESSAGE, time(NULL)));


	assert(handler_list.size() > 0);
	info << "starting" << std::endl;
	while(!done) {
		try {
			// wait for next event
			
			timeout.tv_sec = event_queue.next_timeout() + 2;
			int diff = timeout.tv_sec - time(NULL);
			info << "queue size is " << event_queue.size() << std::endl;
			info << "waiting for next event in " << diff << " seconds" << std::endl;
			if(diff > MAXIMAL_TIMEOUT) {
				timeout.tv_sec = time(NULL) + MAXIMAL_TIMEOUT;
			}
			watchdog.serve();
			if(sem_timedwait(&main_semaphore, &timeout) == 0) {
				info << "woke up from sem_post" << std::endl;
			}
			else {
				switch(errno) {
					case ETIMEDOUT:
						info << "woke up from timeout" << std::endl;
						break;
					case EINTR:
						info << "woke up from signal" << std::endl;
						break;
					default:
						info << "dunno why I woke up" << std::endl;
						break;
				}
			}
			
			// now take care of all events
			watchdog.serve();
			while(event_queue.has_pending()) {
				handle_event(event_queue.pop_event());
			}
		}
		catch(Exception& e) {
			error << "Error: " << e.what() << std::endl;
		}
		catch(...) {
			error << "Unknown Error" << std::endl;
		}
		/// @todo: sanity: make sure a forecast event is in the queue
	}
	return 0;
}

void MessageManager::handle_event(const Event& e)
{
	switch(e.type) {
		case Event::SEND_STATUS_MESSAGE:
			handle_status_message(e);
			break;
		case Event::FETCH_COMMAND_MESSAGE:
			handle_command_message(e);
			break;
	}
}

void MessageManager::handle_status_message(Event e)
{
	bool success = false;
	if(!pm.is_allowed(PowerManager::SEND_STATUS)) {
		info << "sending status message not allowed by PowerManager" << std::endl;
		return;
	}
	
	info << "sending status message using " << best_handler()->get_name() << std::endl;
	try {
		success = best_handler()->send_short_message(status_message.read());
	}
	catch(Exception& e) {
		error << "sending of short message failed: " << e.what() << std::endl;
	}
	catch(...) {
		error << "sending of short message failed" << std::endl;
	}

	if(success) {
		info << "status message sent" << std::endl;
	}
	else {
		e.tries++;
		if(e.tries >= STATUS_MESSAGE_RETRY_LIMIT) {
			info << "retry limit reached, dropping message" << std::endl;
		}
		else {
			info << "resceduling status message" << std::endl;
			e.time = time(NULL) + STATUS_MESSAGE_RETRY_PERIOD;
			event_queue.push_event(e);
		}
	}
}

void MessageManager::handle_command_message(Event e)
{
	if(command_message.read() != "") {
		info << "last message hasn't been read, not fetching new one" << std::endl;
		return;
	}
	std::string message;
	if(!pm.is_allowed(PowerManager::FETCH_COMMAND)) {
		info << "polling command message not allowed by PowerManager" << std::endl;
		return;
	}
	
	info << "polling command message using " << best_handler()->get_name() << std::endl;
	try {
		message = best_handler()->poll_short_message();
		if(message.length() == 0) {
			info << "no message pending" << std::endl;
		}
		else {
			info << "received message: " << message << std::endl;
			command_message.write(message);
		}
	}
	catch(Exception& e) {
		error << "polling of command message failed: " << e.what() << std::endl;
	}
	catch(...) {
		error << "polling of command message failed" << std::endl;
	}

	e.time = time(NULL) + COMMAND_MESSAGE_PERIOD;
	event_queue.push_event(e);
}

TransportHandler *MessageManager::best_handler()
{
	// if it is already connected, don't touch it
	if(current_handler && current_handler->is_connected()) {
		return current_handler;
	}
	// else take the cheapest one that is available
	for(std::vector<TransportHandler*>::iterator i = handler_list.begin(); i != handler_list.end(); i++) {
		if((*i)->is_available()) {
			current_handler = *i;
			return current_handler;
		}
	}
	// no handler will connect, we might as well return the first one
	return *handler_list.begin();
}

/**
 * Send a status message back home. This function can be called asynchronously from other threads.
 *
 * @param status	The message to be sent back home
 * @return		MessageHandler::RequestStatus::OK the message was sucessfully scheduled to be sent,
 *			MessageHandler::RequestStatus::FAIL otherwise.
 */
MessageHandler::RequestStatus MessageManager::send_status(const std::string &status)
{
	try {
		status_message.write(status);
		event_queue.remove_pending(Event::SEND_STATUS_MESSAGE);
		event_queue.push_event(Event(Event::SEND_STATUS_MESSAGE, time(NULL)));
		if(sem_post(&main_semaphore) != 0) {
			return FAIL;
		}
	} catch (...) {
		return FAIL;
	}
	return OK;
}

/**
 * Polls for a command message. This function can be called asynchronously from other threads.
 * Every poll clears a pending message.
 *
 * @param message	Reference to a message variable used for returning the message, if any
 * @return		MessageHandler::RequestStatus::OK if a new message was received,
 *			MessageHandler::RequestStatus::FAIL otherwise.
 */
MessageHandler::RequestStatus MessageManager::receive_message(std::string &message)
{
	try {
		message = command_message.read_write("");
		if(message.length() == 0) {
			return FAIL;
		}
	} catch (...) {
		return FAIL;
	}
	return OK;
}

void MessageManager::register_transport_handler(TransportHandler *handler)
{
	handler_list.insert(lower_bound(handler_list.begin(), handler_list.end(), handler, TransportHandler::compare), handler);
}

MessageManager::EventQueue::EventQueue()
{
	pthread_mutex_init(&mutex, NULL);
}

MessageManager::EventQueue::~EventQueue()
{
	pthread_mutex_destroy(&mutex);
}

void MessageManager::EventQueue::push_event(const Event &e)
{
	if(pthread_mutex_lock(&mutex) != 0) {
		throw(Exception("locking EventQueue mutex"));
	}
	queue.push_back(e);
	push_heap(queue.begin(), queue.end());
	if(pthread_mutex_unlock(&mutex) != 0) {
		throw(Exception("unlocking EventQueue mutex"));
	}
	
}

MessageManager::Event MessageManager::EventQueue::pop_event()
{
	if(pthread_mutex_lock(&mutex) != 0) {
		throw(Exception("locking EventQueue mutex"));
	}
	Event ret = queue.front();
	pop_heap(queue.begin(), queue.end());
	// after pop_heap, the heap is one element shorter
	queue.pop_back();
	if(pthread_mutex_unlock(&mutex) != 0) {
		throw(Exception("unlocking EventQueue mutex"));
	}
	return ret;
	
}

bool MessageManager::EventQueue::has_pending()
{
	bool ret = false;
	if(pthread_mutex_lock(&mutex) != 0) {
		throw(Exception("locking EventQueue mutex"));
	}
	if(queue.empty()) {
		ret = false;
	}
	else if(queue.front().time <= time(NULL)) {
		ret = true;
	}
	if(pthread_mutex_unlock(&mutex) != 0) {
		throw(Exception("unlocking EventQueue mutex"));
	}
	return ret;
}

int MessageManager::EventQueue::next_timeout()
{
	int timeout = 0;
	if(pthread_mutex_lock(&mutex) != 0) {
		throw(Exception("locking EventQueue mutex"));
	}
	if(queue.empty()) {
		if(pthread_mutex_unlock(&mutex) != 0) {
			throw(Exception("unlocking EventQueue mutex"));
		}
		throw(Exception("Event Queue is empty!"));
	}
	timeout = queue.front().time;
	if(pthread_mutex_unlock(&mutex) != 0) {
		throw(Exception("unlocking EventQueue mutex"));
	}
	if(timeout < 0) {
		return 1;
	}

	return timeout;
}




/**
 * Removes any pending Event of type t
 */
int MessageManager::EventQueue::remove_pending(MessageManager::Event::Type t)
{
	int removed = 0;
	if(pthread_mutex_lock(&mutex) != 0) {
		throw(Exception("locking EventQueue mutex"));
	}
	for(std::vector<Event>::iterator i = queue.begin(); i != queue.end(); i++) {
		if(i->type == t) {
			queue.erase(i);
			removed++;
		}
	}
	if(removed > 0) {
		make_heap(queue.begin(), queue.end());
	}
	if(pthread_mutex_unlock(&mutex) != 0) {
		throw(Exception("unlocking EventQueue mutex"));
	}
	return removed;
}
