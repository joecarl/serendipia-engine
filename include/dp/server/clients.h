
#ifndef CLIENTS_H
#define CLIENTS_H

#include <dp/udpcontroller.h>
#include <dp/connectionhandler.h>
#include <dp/neteventslistenershandler.h>
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <queue>
#include <vector>

namespace dp::server {
	class BaseServer;
}

namespace dp::server {

struct EventListener {
	std::string evt_name;
	std::function<void()> cb;
};




class Client : public ConnectionHandler {

	static uint64_t count_instances;

	BaseServer* server;

	bool app_validated = false;

	bool validate_app_info(boost::json::object& data);

	bool preprocess_pkg(NetPackage& pkg);

	NetEventsListenersHandler* nelh;

public:

	boost::json::object cfg;

	Client(BaseServer* _server, boost::asio::ip::tcp::socket&& _socket);
	
	//~Client();

	NetEventsListenersHandler* get_nelh() { return this->nelh; }

};

} // namespace dp::server

#endif
