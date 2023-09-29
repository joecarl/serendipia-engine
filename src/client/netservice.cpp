
#include <dp/client/netservice.h>
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

NetService::NetService(BaseClient* _client) : 
	client(_client),
	socket(io_context),
	udp_controller(nullptr),
	udp_channel(nullptr)
{

}


int64_t time_ms() {
	
	/*
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	return now_ms.time_since_epoch().count()
	*/

	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();

}


void NetService::start_ping_thread() {

	auto cb = [this] (boost::json::object& obj) {
 
		this->ping_ms = time_ms() - obj["ms"].to_number<int64_t>();
		//std::cout << "PING: " << this->ping_ms << "ms" << endl;

	};

	boost::thread([this, cb] {

		while (true) {

			this->send_request("net/ping", { {"ms", time_ms()} }, cb);
			
			std::this_thread::sleep_for(std::chrono::seconds(1));

		}

	});

}


void NetService::connect(const string& host, unsigned short port) {

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
			this->qread();
			this->send_app_info();
			this->io_context.run();

		} catch (std::exception &e) {

			cout << "Error occurred[C" << this->id_client << "]: " << e.what() << endl;
			this->connection_state = CONNECTION_STATE_DISCONNECTED;

		}

	});

}


void NetService::send_app_info() {

	auto& app_info = this->client->app_info;

	boost::json::object pkg = {
		{"type", "appInfo"},
		{"appVersion", app_info.version},
		{"appName", app_info.name},
		{"appPkgname", app_info.pkgname},
		//{"appCfg", this->engine->get_cfg()}
	};
	// this is the first tcp pkg which must be sent to the server, before 
	// udp channel is set. (ping packages are accepted)
	// the server will validate this package and will use it to determine 
	// whether to continue accepting packages or drop the connection
	this->qsend(boost::json::serialize(pkg));	

}


NetService::~NetService() {

	this->io_context.stop();
	
	delete this->udp_controller;

	//std::this_thread::sleep_for(std::chrono::seconds(1));
	
}

ConnState NetService::get_state() {

	return this->connection_state;

}


void NetService::setup_udp(string& local_id) {

	auto tcp_local_ep = this->socket.local_endpoint();
	auto tcp_remote_ep = this->socket.remote_endpoint();

	udp::endpoint local_endpoint(tcp_local_ep.address(), tcp_local_ep.port());
	udp::endpoint remote_endpoint(tcp_remote_ep.address(), tcp_remote_ep.port());
	this->udp_controller = new UdpController(this->socket.get_executor(), local_endpoint, local_id);
	this->udp_channel = this->udp_controller->create_channel(remote_endpoint, "SERVER").get();
	this->udp_channel->send_handshake();

	this->udp_channel->process_pkg_fn = [this] (boost::json::object& pkg) {
		this->handle_json_pkg(pkg);
	};

	this->connection_state = CONNECTION_STATE_CONNECTED_FULL;

}


const string& NetService::get_local_id() {

	if (this->udp_controller == nullptr) {
		throw std::runtime_error("call to NetService::get_local_id before connection is fully established");
	}

	return this->udp_controller->local_id;
	
}


void NetService::qsend_udp(const std::string& pkg) {
	
	if (this->udp_channel == nullptr) {
		cerr << "Cannot send pkg, upd channel not established yet" << endl;
		return;
	}

	this->udp_channel->send(pkg);

	pkgs_sent++;

}

void NetService::qsend(const std::string& pkg) {
	
	if (this->busy) {
		this->pkg_queue.push(pkg);
		return;
	}

	this->busy = true;
/*
	auto handler = boost::bind(&NetService::handle_qsent_content, this,
							   asio::placeholders::error(),
							   asio::placeholders::bytes_transferred());
*/
	//pkg += "\r\n\r\n";
	
	auto sendbuf = std::make_shared<std::string>(pkg + "\r\n\r\n");

	auto handler = [this, sendbuf] (const boost::system::error_code& error, std::size_t bytes_transferred) {
		this->handle_qsent_content(error, bytes_transferred);
		sendbuf;// TODO: will it be freed?
	};

	asio::async_write(socket, asio::buffer(*sendbuf), handler);
	
	pkgs_sent++;

}

void NetService::qread() {

	auto handler = [this] (const boost::system::error_code& error, std::size_t bytes_transferred) {
		this->handle_qread_content(error, bytes_transferred);
	};

	socket.async_read_some(asio::buffer(read_buffer, 1024), handler);

}

void NetService::handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	this->busy = false;

	if (error) {
		
		throw std::runtime_error(error.message());

	}

	if (!this->pkg_queue.empty()) {
		this->qsend(this->pkg_queue.front());
		this->pkg_queue.pop();
	}

}

void NetService::handle_json_pkg(boost::json::object& obj) {
	//cout << pkg << endl;
	/*
	if (pt["binary_transfer"]) {//as bool
		this->wait_for_binary = true;
		this->wait_for_binary_pt = pt;
	}
	*/

	if (obj.contains("id")) { // means its a request response

		uint64_t id = obj["id"].to_number<uint64_t>();
		auto cb_it = this->requests_cbs.find(id);
		if (cb_it != this->requests_cbs.end()) {
			cb_it->second(obj["data"].as_object());
		}

	} else if (obj["type"].is_string()) { // means its a normal event
		//scoped_type
		string type = obj["type"].get_string().c_str();
		boost::json::object data = obj["data"].as_object();

		if (type == "set_client_id") {
			
			string id = obj["client_id"].as_string().c_str();
			cout << "SETTING UP UDP. CLIENT ID: " << id << endl;
			this->setup_udp(id);

		} 
		
		for (auto& nelh: this->nelhs) {
			const auto& listeners = (*nelh).get_listeners(type);
			for (auto& l: listeners) {
				l->cb(data);
			}
		}
	}
}


void NetService::handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	if (error) {
		throw std::runtime_error(error.message());
	}

	std::string data((char*) read_buffer, bytes_transferred);
	
	data = read_remainder + data;
	std::string pkg;

	while ((pkg = extract_pkg(data)) != "") {
		pkgs_recv ++;
		//std::cout << " R:" << pkgs_recv << endl;

/*
		if (this->binary_request_response) {
			uint64_t id = this->binary_request_response;
			auto cb_it = this->requests_cbs.find(id);
			if (cb_it != this->requests_cbs.end()) {
				cb_it->second(pkg);
			}
			this->binary_request_response = 0;

		} else {
*/
			boost::json::value pt = boost::json::parse(pkg);

			if (pt.is_object()) {

				boost::json::object obj = pt.get_object();
				this->handle_json_pkg(obj);

			} else {

				cout << "error parsing: " << pkg << endl;

			}
//		}
	}
	read_remainder = data;
	//cout << read_remainder << endl;
	qread();
}

void NetService::send_request(const std::string& type, const boost::json::object& data, const CallbackFnType& _cb) {

	uint64_t id = this->next_req_id++;
	boost::json::object pkg = {
		{"id", id},
		{"type", type},
		{"data", data}
	};

	this->requests_cbs[id] = _cb;

	this->qsend(boost::json::serialize(pkg));

}

void NetService::send_event(const std::string& type, const boost::json::object& data) {
	
	boost::json::object pkg = {
		{"type", type},
		{"data", data}
	};

	this->qsend_udp(boost::json::serialize(pkg));

}


NetEventsListenersHandler* NetService::create_nelh() {

	auto nelh = std::make_unique<NetEventsListenersHandler>();
	auto nelh_ptr = nelh.get();
	this->nelhs.push_back(std::move(nelh));

	return nelh_ptr;

}

//#include <algorithm>
bool NetService::remove_nelh(NetEventsListenersHandler* nelh) {

	auto iter = this->nelhs.begin();

	while (iter != this->nelhs.end()) {
		if (iter->get() == nelh) { 
			this->nelhs.erase(iter);
			return true;
		}
		iter++;
	}

	return false;

}

} // namespace dp::client
