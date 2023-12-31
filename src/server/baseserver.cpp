#include <dp/server/baseserver.h>
#include <dp/server/group.h>
#include <dp/utils.h>

#include <ctime>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using std::cout;
using std::cerr;
using std::endl;

namespace dp::server {

BaseServer::BaseServer(const AppInfo& _app_info, uint16_t _port): 
	port(_port),
	io_context(), 
	acceptor(io_context),
	tcp_local_endpoint(tcp::v4(), port),
	udp_local_endpoint(udp::v4(), port),
	udp_controller(io_context.get_executor(), udp_local_endpoint, "SERVER"),
	app_info(_app_info)
{

	this->start_listening();

	this->udp_controller.on_new_channel = [this] (UdpChannelController& ch) {
		std::string ch_id = ch.get_remote_id();
		if (this->clients.find(ch_id) == this->clients.end()) {
			cerr << "BAD channel ID: " << ch_id << endl;
			return;
		}
		
		auto cl = this->clients[ch_id];
		cl->set_udp_channel(&ch);
		
	};

}


void BaseServer::remove_client(const std::string& idx) {

	cout << "Removig client" << idx << endl;
	delete clients[idx];
	//clients[idx] = nullptr;
	clients.erase(idx);

}

void BaseServer::start_listening() {

	boost::system::error_code ec;

	if (!this->acceptor.is_open()) {
		cout << "Opening acceptor " << this->tcp_local_endpoint << endl;
		this->acceptor.open(this->tcp_local_endpoint.protocol());
		this->acceptor.set_option(tcp::acceptor::reuse_address(true));
		this->acceptor.bind(this->tcp_local_endpoint);
		this->acceptor.listen();
	}

}


void BaseServer::on_new_connection(tcp::socket& socket) {

	cout << "[" << date() << "] New connection!" << endl;

	if (clients.size() >= this->max_connections) {
		cerr << "Unable to handle connection, max_connections reached" << endl;
		return;
	}

	auto cl = new Client(this, std::move(socket));
	const std::string id = cl->get_id();
	clients[id] = cl;
	cl->start_receive();
	
	cl->get_nelh()->add_event_listener("client/login", [this, cl] (const Object& data) {
		cl->cfg = data["cfg"];
		cout << "Client login: " << cl->cfg << endl;
		this->assign_client_to_group(cl);
	});
	
	std::cout << "Client " << id << " connected!" << std::endl;

}


void BaseServer::assign_client_to_group(Client* cl) {

	Group* gr_ptr = nullptr;
	
	for (auto& gr: this->groups) {
		if (gr.second->is_full()) {
			continue;
		}
		gr_ptr = gr.second;
		break;
	}

	if (gr_ptr == nullptr) {
		gr_ptr = new Group(this);
		std::string gid = "G" + std::to_string(gr_ptr->get_id());
		this->groups[gid] = gr_ptr;
		std::string pkg = ConnectionHandler::pkg_to_raw_data({
			.type = "groups/create",
			.data = {
				{"group", {
					{"id", gid},
					{"owner_name", "TEST"},
				}}
			}
		});
		this->broadcast(pkg);
	}

	gr_ptr->add_client(cl);

	cl->send_event("groups/join", {
		{"group", {
			{"id", "G" + std::to_string(gr_ptr->get_id())},
			{"owner_id", gr_ptr->get_owner_id()},
			{"members", gr_ptr->get_members_json()},
		}}
	});

}


void BaseServer::broadcast(const std::string& pkg) {

	for (auto& cl_iter: this->clients) {
		auto& cl = cl_iter.second;
		if (cl != nullptr && cl->is_connected()) {
			//if (cl->logged || 1) {
				cl->qsend_udp(pkg);
			//}
		}
	}

}


void BaseServer::remove_dead_connections() {

	std::vector<std::string> to_remove;
	for (auto& cl: this->clients) {

		if (cl.second != nullptr && cl.second->get_state() == CONNECTION_STATE_DISCONNECTED) {
			to_remove.push_back(cl.first);
		}
	}

	for (auto& k: to_remove) {
		//get rid of dead connections
		this->remove_client(k);	
	}

}


void BaseServer::wait_for_connection() {

	this->remove_dead_connections();

	cout << "Waiting for connection..." << endl;

	acceptor.async_accept([this] (boost::system::error_code ec, tcp::socket socket) {

		if (ec) {
			cout << "Error opening socket: " << ec << endl;
		} else {
			this->on_new_connection(socket);
		}

		this->wait_for_connection();

	});
		
}


void BaseServer::run() {
	
	this->wait_for_connection();
	
	this->io_context.run();

}

} // namespace dp::server