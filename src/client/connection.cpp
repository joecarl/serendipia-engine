
#include <dp/client/connection.h>
#include <dp/client/baseclient.h>
#include <dp/udpcontroller.h>
#include <dp/utils.h>

#include <iostream>
#include <chrono>
#include <thread>

namespace asio = boost::asio;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace dp::client {

Connection::Connection(BaseClient* _client) : 
	ConnectionHandler(),
	client(_client),
	udp_controller(nullptr)
{
	
}

Connection::~Connection() {

	this->io_context.stop();
	delete this->udp_controller;

}

void Connection::connect(const string& host, unsigned short port) {

	if (this->connection_state > CONNECTION_STATE_DISCONNECTED) {
		cerr << "Socket must be disconnected before connecting again" << endl;
		return;
	}

	this->connection_state = CONNECTION_STATE_CONNECTING;
	
	boost::thread([=] {

		try {

			// Create a resolver
			tcp::resolver resolver(this->io_context);

			// Resolve query
			cout << "Resolving " << host << " ..." << endl;
			auto it = resolver.resolve(host, std::to_string(port));

			// resolver.resolve returns an iterator with all resolutions found,
			// we will just select the first one:
			tcp::endpoint endpoint = it->endpoint();
					
			cout << "Trying " << endpoint << " ..." << endl;

			this->socket.connect(endpoint);
			this->connection_state = CONNECTION_STATE_CONNECTED;
			this->current_host = host;

			cout << "Connected to " << this->socket.remote_endpoint() << " !" << endl;

			this->start_ping_thread();
			this->start_receive();
			this->send_app_info();
			this->io_context.run();

		} catch (std::exception &e) {

			cout << "Error occurred[C]: " << e.what() << endl;
			this->connection_state = CONNECTION_STATE_DISCONNECTED;

		}

	});

}


void Connection::setup_udp(string& local_id) {

	auto tcp_local_ep = this->socket.local_endpoint();
	auto tcp_remote_ep = this->socket.remote_endpoint();

	udp::endpoint local_endpoint(tcp_local_ep.address(), tcp_local_ep.port());
	udp::endpoint remote_endpoint(tcp_remote_ep.address(), tcp_remote_ep.port());
	this->udp_controller = new UdpController(this->socket.get_executor(), local_endpoint, local_id);
	
	auto udp_ch = this->udp_controller->create_channel(remote_endpoint, "SERVER").get();
	udp_ch->send_handshake();
	this->set_udp_channel(udp_ch);

}


const string& Connection::get_local_id() {

	if (this->udp_controller == nullptr) {
		throw std::runtime_error("call to Connection::get_local_id before connection is fully established");
	}

	return this->udp_controller->local_id;
	
}


void Connection::send_app_info() { 

	auto& app_info = this->client->app_info;

	boost::json::object data = {
		{"appVersion", app_info.version},
		{"appName", app_info.name},
		{"appPkgname", app_info.pkgname},
		//{"appCfg", this->engine->get_cfg()}
	};
	// this is the first tcp pkg which must be sent to the server, before 
	// udp channel is set. (ping packages are accepted)
	// the server will validate this package and will use it to determine 
	// whether to continue accepting packages or drop the connection
	this->send_event("net/appInfo", data);	

}


bool Connection::preprocess_pkg(NetPackage& pkg) {

	if (pkg.type == "net/set_client_id") {
		
		string id = pkg.data["client_id"].as_string().c_str();
		cout << "SETTING UP UDP. CLIENT ID: " << id << endl;
		this->setup_udp(id);
		this->id = id;

	}

	return true;

}



} // namespace dp::client
