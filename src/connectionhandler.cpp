
#include <dp/connectionhandler.h>
#include <dp/udpcontroller.h>
#include <dp/utils.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

namespace asio = boost::asio;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace dp {

std::string ConnectionHandler::pkg_to_raw_data(const NetPackage& pkg) {

	Object json_pkg = {
		{"type", pkg.type},
		{"data", pkg.data.json()}
	};
	// TODO: add id?

	return json_pkg.serialize();

}

ConnectionHandler::ConnectionHandler() : 
	udp_channel(nullptr),
	socket(nullptr)
{

}

ConnectionHandler::ConnectionHandler(tcp::socket&& _socket) :
	socket(std::make_unique<tcp::socket>(std::move(_socket)))
{
	// TODO: check socket state to set connection_state ?
	this->connection_state = CONNECTION_STATE_CONNECTED;
}

ConnectionHandler::~ConnectionHandler() {
	
	// Remove udp channel?
	
}


void ConnectionHandler::close() {

	this->connection_state = CONNECTION_STATE_DISCONNECTED;
	this->receiving = false;
	this->busy = false;
	this->id = "";
	//this->binary_data = {};
	//this->binary_pending_bytes = 0;
	if (this->udp_channel) {
		this->udp_channel->on_handshake_done = nullptr;
		this->udp_channel->process_pkg_fn = nullptr;
		this->udp_channel = nullptr;
		//this->udp_channel->udp_controller->remove_channel(this->udp_channel);
	}
	this->next_req_id = 1;
	if (this->socket) {
		this->socket->cancel();
	}
	this->dispatch_listeners("net/disconnect");

}


void ConnectionHandler::set_udp_channel(UdpChannelController* ch) {
	
	//cout << "!! Setting UdpChannelController" << endl;
	this->udp_channel = ch;

	this->udp_channel->process_pkg_fn = [this] (boost::json::object& pkg) {
		this->handle_json_pkg(pkg);
	};

	this->udp_channel->on_handshake_done = [this] {
		this->connection_state = CONNECTION_STATE_CONNECTED_FULL;
		cout << "CONNECTION_STATE_CONNECTED_FULL" << endl;
	};

}


ConnState ConnectionHandler::get_state() {

	return this->connection_state;

}


void ConnectionHandler::qsend_udp(const std::string& pkg) {

	//cout << pkg << endl;
	
	if (this->udp_channel == nullptr || this->connection_state != CONNECTION_STATE_CONNECTED_FULL) {
		cerr << "TCP fallback" << endl;
		this->qsend(pkg);
		return;
	}
	
	this->udp_channel->send(pkg);

	pkgs_sent++;

}

void ConnectionHandler::_send(const std::string& pkg) {

	this->busy = true;

	auto sendbuf = std::make_shared<std::string>();

	if (pkg.length() > MAX_BUFFER_SIZE) { // quiza el limite podría ser mayor

		this->pending_data = pkg.substr(MAX_BUFFER_SIZE);
		*sendbuf = pkg.substr(0, MAX_BUFFER_SIZE);

	} else {

		this->pending_data = "";
		*sendbuf = pkg + "\r\n\r\n";

	}
	
	//auto sendbuf = std::make_shared<std::string>(pkg + "\r\n\r\n");

	auto handler = [this, sendbuf] (const boost::system::error_code& error, std::size_t bytes_transferred) {
		this->handle_qsent_content(error, bytes_transferred);
		//cerr << "SENT: " << *sendbuf << endl;
		sendbuf->clear();// TODO: will it be freed?
	};

	asio::async_write(*socket, asio::buffer(*sendbuf), handler);
	
} 

void ConnectionHandler::qsend(const std::string& pkg) {
	
	if (this->busy) {
		this->pkg_queue.push(pkg);
		return;
	}

	this->_send(pkg);

	pkgs_sent++;

}

void ConnectionHandler::start_receive() {

	if (this->receiving) {
		cerr << "Connection already in receiving state" << endl;
		return;
	}
	
	this->qread();

}

void ConnectionHandler::qread() {

	this->receiving = true;

	auto handler = [this] (const boost::system::error_code& error, std::size_t bytes_transferred) {
		
		this->handle_qread_content(error, bytes_transferred);
	};

	socket->async_read_some(asio::buffer(read_buffer, MAX_BUFFER_SIZE), handler);

}

void ConnectionHandler::handle_qsent_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	if (error) {
		cout << "Error occurred (SEND)[" << this->id << "]: " << error.message() << endl;
		this->close();
		return;
	}

	if (this->pending_data.length() > 0) {
		cout << "bunch " << bytes_transferred << " (" << this->pending_data.length() << " remaining)" << endl;
		this->_send(this->pending_data);
		return;
	}

	this->busy = false;

	if (!this->pkg_queue.empty()) {
		this->qsend(this->pkg_queue.front());
		this->pkg_queue.pop();
	}

}


void ConnectionHandler::process_request(NetPackage& req) {

	Object response_data;
	if (req.type == "net/ping") {
		response_data = req.data;
	} else {
		// no more requests listeners so far
		return;
	}

	Object resp = {
		{"id", req.id},
		{"data", response_data.json()}
	};

	this->qsend(resp.serialize());

}


void ConnectionHandler::handle_json_pkg(const Object& obj) {
	//cout << pkg << endl;
	NetPackage pkg = {
		.id = obj.sget<uint64_t>("id", 0),
		.type = obj.sget<std::string>("type", ""),
		.data = obj["data"],
	};

	bool ok = this->preprocess_pkg(pkg);
	if (!ok) {
		return;
	}

	if (pkg.id > 0) { 

		if (pkg.type != "") { // means it is a request

			this->process_request(pkg);

		} else { // means it is a request response, let's call its callback
		
			auto cb_it = this->requests_cbs.find(pkg.id);
			if (cb_it != this->requests_cbs.end()) {
				cb_it->second(obj["data"]);
			}

		}

	} else { // means its a normal event

		if (pkg.type == "net/binary_transfer") {

			this->binary_pending_bytes = pkg.data["size"];

		}
		
		this->dispatch_listeners(pkg.type, pkg.data);

	}

}


void ConnectionHandler::dispatch_listeners(const std::string& type, const Object& data) {

	for (size_t i = 0; i < this->nelhs.size(); i++) {
		// Can't use range based loop here because the vector can be resized inside the loop
		auto nelh_ptr = nelhs[i].get();
		const auto& listeners = nelh_ptr->get_listeners(type);
		for (auto& l: listeners) {
			l->cb(data);
			if (nelh_ptr->is_disposed()) {
				break;
			}
		}
	}

	std::vector<NetEventsListenersHandler*> nelhs_to_remove;
	for (auto& nelh: this->nelhs) {
		if (nelh->is_disposed()) {
			nelhs_to_remove.push_back(nelh.get());
		}
	}
	for (auto nelh_ptr: nelhs_to_remove) {
		this->_remove_nelh(nelh_ptr);
	}

}


void ConnectionHandler::handle_qread_content(const boost::system::error_code& error, std::size_t bytes_transferred) {

	if (error) {
		cout << "Error occurred (READ)[" << this->id << "]: " << error.message() << endl;
		this->close();
		return;
	}

	//cout << "B:" << bytes_transferred << "C: " << (char*)read_buffer << endl;

	stream_buffer.insert(stream_buffer.end(), read_buffer, read_buffer + bytes_transferred);
	//cout << "B:" << stream_buffer << endl;

	if (this->binary_pending_bytes > 0) {
		uint64_t bytes = std::min(stream_buffer.size(), this->binary_pending_bytes);
		cout << "RECEIVING BINARY DATA: " << bytes << endl;
		auto begin = stream_buffer.begin();
		auto end = stream_buffer.begin() + bytes;
		this->binary_data.insert(binary_data.end(), begin, end);
		stream_buffer.erase(begin, end);
		this->binary_pending_bytes -= bytes;
		if (this->binary_pending_bytes == 0) {
			//this->process_binary();
			//this->binary_dta.clear();
		}
	}

	std::string pkg;

	while ((pkg = extract_pkg(stream_buffer)) != "") {

		//cout << "EXTRACTED PKG: " << pkg << endl;
		
		pkgs_recv ++;
		//std::cout << " R:" << pkgs_recv << endl;
		//this->process_pkg(pkg);

		boost::json::value pt = boost::json::parse(pkg);

		if (pt.is_object()) {

			Object obj = pt.get_object();
			this->handle_json_pkg(obj);

		} else {

			cout << "error parsing: " << pkg << endl;

		}

	}
	//cout << stream_buffer << endl;
	this->qread();
	//this->async_wait_for_data();
}

void ConnectionHandler::send_request(const std::string& type, const Object& data, const CallbackFnType& _cb) {

	uint64_t id = this->next_req_id++;
	Object pkg = {
		{"id", id},
		{"type", type},
		{"data", data.json()}
	};

	this->requests_cbs[id] = _cb;

	this->qsend(pkg.serialize());

}

void ConnectionHandler::send_event(const std::string& type, const Object& data) {
	
	Object pkg = {
		{"type", type},
		{"data", data.json()}
	};

	this->qsend_udp(pkg.serialize());

}


NetEventsListenersHandler* ConnectionHandler::create_nelh() {

	auto nelh = std::make_unique<NetEventsListenersHandler>();
	auto nelh_ptr = nelh.get();
	this->nelhs.push_back(std::move(nelh));

	return nelh_ptr;

}

//#include <algorithm>
bool ConnectionHandler::_remove_nelh(NetEventsListenersHandler* nelh) {

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
