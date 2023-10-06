#include <dp/server/group.h>
#include <dp/server/clients.h>
#include <dp/server/baseserver.h>

using std::cout;
using std::cerr;
using std::endl;

namespace dp::server {

boost::json::object member_to_json(const GroupPlayer& pl) {
	return {
		{"client_id", pl.client->get_id()},
		{"name", pl.client->cfg["playerName"].as_string()},
		{"ready", pl.ready},
	};
}

uint64_t Group::count_instances = 0;

Group::Group(BaseServer* _server) : server(_server) {

	this->id_group = count_instances++;

}


void Group::send_member_update(const GroupPlayer& m) {
	
	this->broadcast_event("group/member_update", {
		{"member", member_to_json(m)}
	});

}


void Group::add_client(Client* cl) {

	if (this->owner_id == "") {
		this->owner_id = cl->get_id();
	}

	if (this->is_full()) {
		return;
	}

	// TODO: see idx todo in group.h
	size_t i = 0;
	bool found = false;
	while (!found) {
		found = true;
		for (auto& pl: this->players) {
			if (pl.second.idx == i) {
				i++;
				found = false;
				break;
			}
		}
	}

	std::string player_idx = cl->get_id();

	GroupPlayer pl = {
		.client = cl,
		.ready = false,
		.idx = i,
	};

	this->broadcast_event("group/member_join", { 
		{"member", member_to_json(pl)} 
	});

	this->players[player_idx] = pl;
	this->sorted_members_ids.push_back(player_idx);
	cout << "Client " << player_idx << " joined group " << this->get_id() << endl;

	auto nelh = cl->get_nelh();

	nelh->add_event_listener("connection/drop", [this, player_idx] (boost::json::object& data) {

		//cout << "onDrop event callback!!" << endl;
		this->players.erase(player_idx);
		// TODO: also remove from sorted_members_ids
		cout << "Player with key " << player_idx << " dropped from group" << endl;
		
	});

	nelh->add_event_listener("group/set_ready_state", [this, player_idx] (boost::json::object& data) {

		cout << "Player ready_state changed!!" << endl;
		this->players[player_idx].ready = data["state"].as_bool();
		this->send_member_update(this->players[player_idx]);

		if (!this->is_full()) {
			return;
		}
		bool all_players_ready = true;
		for (auto& pl: this->players) {
			if (!pl.second.ready) {
				all_players_ready = false;
				break;
			}
		}
		if (all_players_ready) {
			this->start_game();
		}

	});

	nelh->add_event_listener("game/desync", [this, player_idx] (boost::json::object& data) {

		boost::json::object o = {
			{"type", "sync"},
			{"gamevars", this->server->export_game(this->game)}
		};

		cerr << "!! RESYNC " << o << endl;
		this->players[player_idx].client->send_event("game/event", o);

	});

	nelh->add_event_listener("game/event", [this, player_idx] (boost::json::object& data) {
		/*
		uint64_t orig_tick = data["tick"].to_number<uint64_t>();
		uint64_t tick_diff = 0; 

		if (orig_tick < this->game->tick) {
			tick_diff = this->game->tick - orig_tick;
			cout << "Tick diff: " << tick_diff << endl;
		}
		*/

		//cout << "TICK: " << this->game->tick << " | IDX: " << this->players[player_idx].idx << endl;
		//data["tick"] = this->game->tick + 0; //TODO: auto calc tick delay based on clients connection?
		data["player_key"] = this->players[player_idx].idx; //player_idx;
		this->evt_queue.push(data);

		//this->broadcast_event("game/event", data);

	});

}

void Group::process_game_event(boost::json::object &evt) {

	//cout << "processing EVT: " << evt << endl;

	auto evt_type = evt["type"].as_string();
		
	if (evt_type == "set_control_state") {
		
		int control = evt["control"].to_number<int64_t>();
		bool new_state = evt["state"].as_bool();
		int player_idx = evt["player_key"].to_number<int64_t>();
		
		this->game->set_player_control_state(player_idx, control, new_state);

	} else {
		cerr << "Unknown event type: " << evt_type << endl;
	}

}

void Group::start_game() {

	cout << "Starting game in group " << this->get_id() << endl;

	if (io != nullptr) {
		io->stop();
		delete io;
		delete t;
	}

	this->new_game();

	io = new boost::asio::io_context();
	t = new boost::asio::steady_timer(*io, boost::asio::chrono::milliseconds(75 * 1000 / 60)); //el 75 se corresponde al delayer, quiza deberia commonizarse

	t->async_wait(boost::bind(&Group::game_main_loop, this));
	
	boost::json::array players_order;
	for (auto& id: this->sorted_members_ids) {
		players_order.push_back(boost::json::string(id));
	}
	this->broadcast_event("group/game_start", {
		{"seed", this->game->get_rnd_seed()},
		{"players_order", players_order},
	});

	boost::thread(boost::bind(&boost::asio::io_context::run, io));

}

void Group::broadcast(const std::string& pkg) {

	for (auto& pl : this->players) {
		auto cl = pl.second.client;
		if (cl != nullptr && cl->is_connected()) {
			//if (cl->logged || 1) {
				cl->qsend_udp(pkg);
			//}
		}
	}

}

void Group::broadcast_event(const std::string& type, const boost::json::object& data) {

	const std::string raw_data = ConnectionHandler::pkg_to_raw_data({
		.type = type,
		.data = data,
	});
	this->broadcast(raw_data);

}


void Group::game_main_loop() {

	//cout << "game_main_loop" << endl;

	while (this->evt_queue.size() > 0) {

		auto evt = this->evt_queue.front();


		evt["tick"] = this->game->tick;
		this->broadcast_event("game/event", evt);


		uint64_t evt_tick = evt["tick"].to_number<uint64_t>();

		if (evt_tick == this->game->tick) {

			this->process_game_event(evt);
			this->evt_queue.pop();

		} else if (evt_tick < this->game->tick) {

			cerr << evt_tick << " < " << this->game->tick << " !!!!!!!!" << endl;
			throw std::runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}

	this->game->process_tick();

	if (this->game->is_finished()) {
		return;
	}

	useconds_t usec = 1000000 / 60;
	t->expires_at(t->expiry() + boost::asio::chrono::microseconds(usec));
	t->async_wait(boost::bind(&Group::game_main_loop, this));

}


void Group::new_game() {

	delete this->game;
	
	// Vaciamos la cola de eventos
	std::queue<boost::json::object> empty;
	std::swap(this->evt_queue, empty);

	this->game = this->server->create_game();
	//cout << "New game created, tick: " << this->game->tick << endl;
	for (auto& pl: this->players) {
		pl.second.ready = false;
		this->send_member_update(pl.second);
	}

}


uint64_t Group::get_id() { 
	
	return this->id_group;

}


bool Group::is_full() {

	return this->players.size() >= this->max_players;

}


boost::json::array Group::get_members_json() {

	boost::json::array members;
	for (auto& id: this->sorted_members_ids) {
		auto p_iter = this->players.find(id);
		if (p_iter == this->players.end()){
			cerr << "Group member not found: " << id << endl;
			continue;
		}
		members.push_back(member_to_json(p_iter->second));
	}

	return members;
	
}


} // namespace dp::server
