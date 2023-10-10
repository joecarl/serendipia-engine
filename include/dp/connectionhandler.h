#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <dp/udpcontroller.h>
#include <dp/neteventslistenershandler.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/json.hpp>

#include <chrono>
#include <queue>

#define MAX_BUFFER_SIZE (32 * 1024)

namespace dp {

typedef struct {
	uint64_t id;
	std::string type;
	boost::json::object data;
} NetPackage;

enum ConnState {
	CONNECTION_STATE_DISCONNECTED = 0,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTED,
	CONNECTION_STATE_CONNECTED_FULL
};

/**
 * Handles the connection with the server. Enables TCP and UDP communication
 */
class ConnectionHandler {

	uint64_t pkgs_sent = 0;

	uint64_t pkgs_recv = 0;

	uint64_t next_req_id = 1;

	std::queue<std::string> pkg_queue;

	std::string pending_data = "";

	UdpChannelController* udp_channel;

	bool busy = false;

	uint8_t read_buffer[MAX_BUFFER_SIZE];

	std::vector<uint8_t> stream_buffer;

	std::vector<uint8_t> binary_data;

	uint64_t binary_pending_bytes = 0;

	std::unordered_map<uint64_t, CallbackFnType> requests_cbs;

	int64_t ping_ms = 0;

	std::vector<std::unique_ptr<NetEventsListenersHandler>> nelhs;

	bool receiving = false;

	bool _remove_nelh(NetEventsListenersHandler* nelh);

	void handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred);

	void handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred);
	
	void handle_json_pkg(boost::json::object& obj);

	void process_request(NetPackage& req);
	
	void qread();

	void _send(const std::string& pkg);
	
	virtual bool preprocess_pkg(NetPackage& req) { return true; }

protected:

	virtual void send_app_info() { }

	virtual bool validate_app_info(boost::json::object& data) { return true; }

	void dispatch_listeners(const std::string& type, boost::json::object& data);
	// TODO: void dispatch_listeners(const std::string& type, const boost::json::object& data = {});

	std::string id;

	std::unique_ptr<boost::asio::ip::tcp::socket> socket;

	ConnState connection_state = CONNECTION_STATE_DISCONNECTED;

	void start_ping_thread();

	void close();

public:

	static std::string pkg_to_raw_data(const NetPackage& pkg);

	/**
	 * Constructs the instance in client mode
	 */
	ConnectionHandler(/*boost::asio::io_context& io_context*/);

	/**
	 * Constructs the instance in server mode
	 */
	ConnectionHandler(boost::asio::ip::tcp::socket&& _socket);

	~ConnectionHandler();
	
	void set_udp_channel(UdpChannelController* ch);

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
	 * Starts reading and processing incoming data
	 */
	void start_receive();

	/**
	 * Obtains the current ping in milliseconds.
	 */
	int64_t get_ping_ms() { return this->ping_ms; }

	NetEventsListenersHandler* create_nelh();

	bool is_connected() { return this->connection_state >= CONNECTION_STATE_CONNECTED; }

	std::string get_id() { return this->id; }

};

} // namespace dp::client

#endif
