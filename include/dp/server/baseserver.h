#ifndef SERVER_H
#define SERVER_H

#include <dp/server/clients.h>
#include <dp/server/group.h>
#include <dp/serendipia.h>

#include <boost/asio.hpp>

namespace dp::server {

class BaseServer {

	/**
	 * The port where the server will listen (used for TCP and UDP).
	 */
	uint16_t port;

	/**
	 * The execution context.
	 */
	boost::asio::io_context io_context;

	/**
	 * The acceptor which will handle incoming TCP connections.
	 */
	boost::asio::ip::tcp::acceptor acceptor;

	/**
	 * The local endpoint used for tcp connections.
	 */
	boost::asio::ip::tcp::endpoint tcp_local_endpoint;

	/**
	 * The local endpoint where the udp socket will receive data.
	 * When comparing tcp and udp endpoints as strings there should be no 
	 * difference.
	 */
	boost::asio::ip::udp::endpoint udp_local_endpoint;
	
	/**
	 * The controller which will manage udp datagrams
	 */
	UdpController udp_controller;

	std::unordered_map<std::string, Client*> clients;

	std::unordered_map<std::string, Group*> groups;

	uint8_t verbose = 0;

	const uint16_t max_connections = 10;

	void wait_for_connection();

	void on_new_connection(boost::asio::ip::tcp::socket& socket);
	
	void remove_dead_connections();

	void remove_client(const std::string& idx);
	
	void start_listening();

	void broadcast(const std::string& pkg);

	void assign_client_to_group(Client* cl);

public:

	const AppInfo app_info;

	BaseServer(const AppInfo& app_info, uint16_t _port);

	virtual BaseGame* create_game() = 0;

	virtual Object export_game(dp::BaseGame* game) = 0;

	void run();

};

} // namespace dp::server

#endif
