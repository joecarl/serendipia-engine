#ifndef CONNECTION_H
#define CONNECTION_H

#include <dp/connectionhandler.h>
#include <dp/udpcontroller.h>
#include <dp/neteventslistenershandler.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <queue>

namespace dp::client {
	class BaseClient;
}

namespace dp::client {

/**
 * Handles the connection with the server. Enables TCP and UDP communication
 */
class Connection : public ConnectionHandler {

	BaseClient* client;

	bool io_context_running = false;

	boost::asio::io_context io_context;
	
	boost::asio::ip::tcp::resolver resolver;

	boost::asio::steady_timer ping_timer;

	UdpController* udp_controller;
	
	std::string current_host;

	int64_t ping_ms = 0;

	void setup_udp(std::string& local_id);

	void send_app_info();

	bool preprocess_pkg(NetPackage& pkg);
	
	void start_ping_task();

public:

	Connection(BaseClient* client);

	~Connection();

	/**
	 * Connects to the specified address and port, the function always returns
	 * immediately and the state will change to `CONNECTION_STATE_CONNECTING`.
	 * If the process is successful the state will eventually change to 
	 * `CONNECTION_STATE_CONNECTED`. At this point TCP communication is 
	 * available and calling `qsend` should work. After this, a local id will
	 * be received and stored, a UDP channel will be created and when it is 
	 * fully established the state will change to 
	 * `CONNECTION_STATE_CONNECTED_FULL`.
	 * @param addr the endpoint address
	 * @param port the endpoint port
	 */
	void connect(const std::string& addr, unsigned short port);

	/**
	 * Obtains the last host that the client connected to.
	 */
	std::string& get_current_host() { return this->current_host; }

	/**
	 * Retrieves the local id provided by the server. This function should not
	 * be called before the connection state reaches 
	 * `CONNECTION_STATE_CONNECTED_FULL`.
	 */
	const std::string& get_local_id();

	/**
	 * Starts the io_context run function in a separate thread
	 */
	void start_context_thread();

	/**
	 * Obtains the current ping in milliseconds.
	 */
	int64_t get_ping_ms() { return this->ping_ms; }

};

} // namespace dp::client

#endif
