#ifndef NETEVENTSLISTENERSHANDLER_H
#define NETEVENTSLISTENERSHANDLER_H

#include <dp/serendipia.h>
#include <dp/object.h>
#include <boost/json.hpp>
#include <unordered_map>
#include <memory>
#include <vector>

namespace dp::client {
	class BaseClient;
}

namespace dp {

typedef std::function<void(const Object& obj)> CallbackFnType;

typedef struct {
	std::string type;
	CallbackFnType cb;
} NetEventListener;

class NetEventsListenersHandler {

	uint64_t next_listener_id = 1;

	bool disposed = false;
	
	std::unordered_map<uint64_t, NetEventListener> listeners;

public:

	bool enabled = true;

	NetEventsListenersHandler();
	
	~NetEventsListenersHandler() { }

	/**
	 * Registers a callback function which will be called when an event with
	 * type equal to `evt_type` arrives
	 */
	uint64_t add_event_listener(const std::string& evt_type, CallbackFnType cb);

	/**
	 * Removes a previously registered evt listener. Returns the number of 
	 * events removed
	 */
	size_t remove_event_listener(uint64_t id);

	
	std::vector<const NetEventListener*> get_listeners(const std::string& type);

	void dispose() { this->disposed = true; this->enabled = false; }

	bool is_disposed() { return this->disposed; }

};

} // namespace dp

#endif
