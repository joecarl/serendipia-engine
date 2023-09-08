#include <dp/server/group.h>
#include <dp/server/clients.h>
#include <dp/server/baseserver.h>

using std::cout;
using std::cerr;
using std::endl;

namespace dp::server {

uint64_t Group::count_instances = 0;

Group::Group(BaseServer* _server) : server(_server) {

	//this->new_game();
	this->id_group = count_instances++;

}


void Group::add_client(Client* cl) {

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

	std::string player_idx = "C" + std::to_string(cl->get_id());

	GroupPlayer pl = {
		.client = cl,
		.ready = false,
		.idx = i,
	};

	this->players[player_idx] = pl;
	cout << "Client " << player_idx << " joined group " << this->get_id() << endl;

	cl->add_event_listener("onDrop", [this, player_idx] () {

		//cout << "onDrop event callback!!" << endl;
		this->players.erase(player_idx);
		cout << "Player with key " << player_idx << " dropped from group" << endl;
		
	});

	
	cl->on_pkg_received = [this, player_idx] (boost::json::object& pkg) {

		auto evt_type = pkg["type"].as_string();
	
		if (evt_type == "clientConfig") {//if scope == group-event

			cout << "Client config: " << pkg["data"] << endl;
			// en este pkg data hay mucha info innecesaria, quiza se deberia 
			// construir un objecto unevo con lo estrictamente necesario
			auto player_cfg = pkg["data"].as_object();
			player_cfg["clientId"] = player_idx;
			this->players[player_idx].cfg = player_cfg;
			//set

		} else if (evt_type == "ready_to_play") {//if scope == group-event

			cout << "Player ready!!" << endl;
			this->players[player_idx].ready = true;

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

		} else if (evt_type == "desync") {

			boost::json::object o = {
				{"type", "sync"},
				{"gamevars", this->server->export_game(this->game)}
			};

			cerr << "!! RESYNC " << o << endl;
			this->players[player_idx].client->qsend_udp(boost::json::serialize(o));

		} else {

		//if (evt_type == "set_control_state") {//if scope == game-event
			//cout << pkg << endl;

			uint64_t orig_tick = pkg["tick"].to_number<uint64_t>();
			uint64_t tick_diff = 0;

			if (orig_tick < this->game->tick) {
				tick_diff = this->game->tick - orig_tick;
				cout << "Tick diff: " << tick_diff << endl;
			}

			cout << "TICK: " << this->game->tick << " | IDX: " << this->players[player_idx].idx << endl;
			pkg["player_key"] = this->players[player_idx].idx; //player_idx;
			pkg["tick"] = this->game->tick + 0; //TODO: auto calc tick delay based on clients connection?

			this->evt_queue.push(pkg);

			this->send_to_all(boost::json::serialize(pkg));
		//}
		}
	
	};

}

void Group::process_event(boost::json::object &evt) {

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
		//delete io;
		//delete t;
	}

	this->new_game();

	io = new boost::asio::io_context();

	t = new boost::asio::steady_timer(*io, boost::asio::chrono::milliseconds(75 * 1000 / 60)); //el 75 se corresponde al delayer, quiza deberia commonizarse

	t->async_wait(boost::bind(&Group::game_main_loop, this));

	boost::json::array players_cfg;
	for (auto& pl: this->players) {
		players_cfg.push_back(pl.second.cfg);
	}

	boost::json::object pkg = {
		{"type", "gameStart"},
		{"seed", this->game->get_rnd_seed()},
		{"playersCfg", players_cfg}
	};

	this->send_to_all(boost::json::serialize(pkg));

	boost::thread(boost::bind(&boost::asio::io_context::run, io));

}

void Group::send_to_all(const std::string& pkg) {

	for (auto& pl : this->players) {
		auto cl = pl.second.client;
		if (cl != nullptr && !cl->is_dead()) {
			//if (cl->logged || 1) {
				cl->qsend_udp(pkg);
			//}
		}
	}

}


void Group::game_main_loop() {

	//cout << "game_main_loop" << endl;

	while (this->evt_queue.size() > 0) {

		auto evt = this->evt_queue.front();

		//cout << "checking EVT: " << evt << " | is uint64 ? " << evt["tick"].is_uint64() << endl;

		unsigned int evt_tick = (unsigned int)(evt["tick"].as_uint64());

		if (evt_tick == this->game->tick) {

			this->process_event(evt);

			this->evt_queue.pop();

		} else if (evt_tick < this->game->tick) {

			throw std::runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}

	this->game->process_tick();

	if (this->game->is_finished()) {
		
		//this->new_game();

		return;

	}

	useconds_t usec = 1000000 / 60;
	
	t->expires_at(t->expiry() + boost::asio::chrono::microseconds(usec));

	t->async_wait(boost::bind(&Group::game_main_loop, this));

}


void Group::new_game() {

	delete this->game;
	
	//vaciamos la cola de eventos
	std::queue<boost::json::object> empty;
	std::swap(this->evt_queue, empty);

	this->game = this->server->create_game();
	//cout << "New game created, tick: " << this->game->tick << endl;
	for (auto& pl: this->players) {
		pl.second.ready = false;
	}

}


uint64_t Group::get_id() { 
	
	return this->id_group;

}


bool Group::is_full() {

	return this->players.size() >= this->max_players;

}


} // namespace dp::server
