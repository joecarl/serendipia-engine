
#include <dp/serendipia.h>
#include <dp/neteventslistenershandler.h>
#include <boost/json.hpp>
#include <unordered_map>
#include <memory>

namespace dp::client {
	class BaseClient;
}

namespace dp::client {

	
NetEventsListenersHandler::NetEventsListenersHandler() {
	
}

uint64_t NetEventsListenersHandler::add_event_listener(const std::string& evt_type, CallbackFnType cb) {

	uint64_t id = this->next_listener_id++;
	this->listeners[id] = {
		.type = evt_type,
		.cb = cb
	};

	return id;

}

size_t NetEventsListenersHandler::remove_event_listener(uint64_t id) {

	return this->listeners.erase(id);

}


std::vector<const NetEventListener*> NetEventsListenersHandler::get_listeners(const std::string& type) {

	std::vector<const NetEventListener*> out;
	
	if (!this->enabled) {
		return out;
	}

	for (auto& l: this->listeners) {
		if (l.second.type == type) {
			out.push_back(&(l.second));
		}
	}

	return out;

}

}