#include <dp/client/onlinegamecontroller.h>
#include <iostream>

using std::cout, std::cerr, std::endl;

namespace dp::client {

void OnlineGameController::push_event(const Object &evt) {

	auto evt_type = evt.sget<std::string>("type");

	if (evt_type == "sync") {
		this->sync_data = evt;
		return;
	} 

	this->evt_queue.push(evt);
	
}

void OnlineGameController::process_event(const Object &evt) {

	//cout << "Processing evt" << evt << endl;

	auto evt_type = evt.sget<std::string>("type");
		
	if (evt_type == "set_control_state") {

		int control = evt["control"]; 
		bool new_state = evt["state"];
		int player_key = evt["player_key"];
		this->game->set_player_control_state(player_key, control, new_state);

	} else if (evt_type == "set_paused_state") {

		bool new_state = evt["state"];
		this->game->paused = new_state;

	} else {

		cerr << "Unknown event type: " << evt_type << endl;

	}

}

void OnlineGameController::setup(BaseGame *game) {
	
	this->game = game;
	
	//vaciamos la cola de eventos
	std::queue<dp::Object> empty;
	std::swap(this->evt_queue, empty);

}


void OnlineGameController::process_sync_data() {

	const Object& gamevars = this->sync_data["gamevars"];
	uint64_t tick = gamevars["tick"];

	// Eliminamos todos los eventos que ya no haya que procesar
	while (this->evt_queue.size() > 0) {
		auto& next_evt = this->evt_queue.front();
		uint64_t next_evt_tick = next_evt["tick"];
		if (next_evt_tick >= tick) {
			break;
		}
		this->evt_queue.pop();
	}

	//cerr << "!! SYNCING: " << this->sync_data << endl;
	try {
		if (this->sync_game == nullptr) {
			throw std::logic_error("sync_game function is not defined!");
		}
		this->sync_game(this->game, gamevars);
	} catch (std::logic_error& e) {
		cerr << "SYNC ERROR: " << e.what() << endl;
	}

	this->sync_data = {};

}


void OnlineGameController::on_tick() {

	if (!this->sync_data.json().empty()) {
		this->process_sync_data();
	}

	while (this->evt_queue.size() > 0) {

		auto& evt = this->evt_queue.front();
		uint64_t evt_tick = evt["tick"];

		if (evt_tick == this->game->tick) {

			this->process_event(evt);
			this->evt_queue.pop();

		} else if (evt_tick < this->game->tick) {

			cerr << "DESYNC! evt_tick: " << evt_tick << " | game_tick: " << this->game->tick << endl;
			this->evt_queue.pop();
		
			throw std::runtime_error("Evento perdido");

		} else {
			
			break;

		}

	}
}

} // namespace dp::client
