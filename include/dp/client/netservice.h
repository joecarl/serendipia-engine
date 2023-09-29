//
//  netservice.h
//  dp
//
//  Created by Joe on 13/9/18.
//

#ifndef NETSERVICE_H
#define NETSERVICE_H

#include <dp/udpcontroller.h>
#include <dp/client/neteventslistenershandler.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <queue>

namespace dp::client {
	class BaseClient;
}

namespace dp::client {

enum ConnState {
	CONNECTION_STATE_DISCONNECTED = 0,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTED,
	CONNECTION_STATE_CONNECTED_FULL
};

/**
 * Handles the connection with the server. Enables TCP and UDP communication
 */
class NetService {

	BaseClient* client;

	uint64_t pkgs_sent = 0;

	uint64_t pkgs_recv = 0;

	uint64_t next_req_id = 1;

	std::queue<std::string> pkg_queue;

	boost::asio::io_context io_context;
	
	boost::asio::ip::tcp::socket socket;

	UdpController* udp_controller;

	UdpChannelController* udp_channel;

	bool busy = false;

	ConnState connection_state = CONNECTION_STATE_DISCONNECTED;

	int id_client = 0;

	unsigned int pkg_id = 0;

	unsigned char read_buffer[1024];

	std::string read_remainder = "";

	bool wait_for_binary = false;

	boost::json::object wait_for_binary_pt;

	std::unordered_map<uint64_t, CallbackFnType> requests_cbs;

	void send_app_info();

	void setup_udp(std::string& local_id);

	void start_ping_thread();

	void handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred);

	void handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	
	void handle_json_pkg(boost::json::object& obj);
	
	void qread();

	int64_t ping_ms = 0;

	std::string current_host;

	std::vector<std::unique_ptr<NetEventsListenersHandler>> nelhs;

public:

	NetService(BaseClient* client);

	~NetService();

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
	 * Obtains the connection state.
	 */
	ConnState get_state();

	/**
	 * Sends a request which should be responded. Will always be sent via TCP
	 * @param type a scoped type in the format <scope>/<type>
	 * @param data random json object
	 * @param _cb the callback function which will be executed when the 
	 * response arrives
	 */
	void send_request(const std::string& type, const boost::json::object& data = {}, const CallbackFnType& _cb = nullptr);

	/**
	 * Sends an event which won't receive any response. Will be sent via UDP
	 * @param type a scoped type in the format `<scope>/<type>`
	 * @param data random json object
	 */
	void send_event(const std::string& type, const boost::json::object& data = {});

	/**
	 * Sends raw data via TCP socket.
	 */
	void qsend(const std::string& pkg);

	/**
	 * Sends raw data via UDP socket.
	 */
	void qsend_udp(const std::string& pkg);

	/**
	 * Retrieves the local id provided by the server. This function should not
	 * be called before the connection state reaches 
	 * `CONNECTION_STATE_CONNECTED_FULL`.
	 */
	const std::string& get_local_id();


	/**
	 * Obtains the last host that the client connected to.
	 */
	std::string& get_current_host() { return this->current_host; }

	/**
	 * Obtains the current ping in milliseconds.
	 */
	int64_t get_ping_ms() { return this->ping_ms; }

	NetEventsListenersHandler* create_nelh();

	bool remove_nelh(NetEventsListenersHandler* nelh);

};

} // namespace dp::client

#endif
