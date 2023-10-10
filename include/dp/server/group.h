#ifndef GROUP_H
#define GROUP_H

#include <ctime>
#include <vector>
#include <queue>
#include <iostream>
#include <stdlib.h>

#include <boost/json.hpp>
#include <boost/chrono.hpp>
#include <boost/asio.hpp>

namespace dp {
	class BaseGame;
	namespace server {
		class Client;
		class BaseServer;
	}
}

namespace dp::server {

typedef struct {
	
	Client* client;

	bool ready;

	boost::json::object cfg;

	// TODO: (pensar) en lugar de este idx usar una funcion register_player en la clase BaseGame? que reciba un key string y lo asigne a un player y devuelva el indice asignado?
	uint64_t idx;

} GroupPlayer;

class Group {
	
	static uint64_t count_instances;

	uint64_t id_group;

	BaseServer* server;

	BaseGame* game = nullptr;

	std::string owner_id;

	std::unordered_map<std::string, GroupPlayer> players;

	std::vector<std::string> sorted_members_ids;

	std::queue<boost::json::object> evt_queue;

	boost::asio::io_context *io = nullptr;

	boost::asio::steady_timer *t;

	uint64_t max_players = 2;

	void game_main_loop();

	void process_game_event(boost::json::object &evt);

	void send_member_update(const GroupPlayer& m);

public:

	Group(BaseServer* _server);

	uint64_t get_id();

	bool is_full();
	
	void new_game();

	void add_client(Client* cl);
	
	void drop_member(const std::string& id);

	void broadcast(const std::string& pkg);

	void broadcast_event(const std::string& type, const boost::json::object& data);
	
	void start_game();

	const std::string& get_owner_id() { return this->owner_id; }
	
	boost::json::array get_members_json();

};

} // namespace dp::server

#endif
