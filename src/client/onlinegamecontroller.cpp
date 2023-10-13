#include <dp/client/onlinegamecontroller.h>
#include <iostream>

using std::cout, std::cerr, std::endl;

namespace dp::client {

void OnlineGameController::push_event(const Object &evt) {

	std::lock_guard<std::mutex> guard(this->evt_queue_mutex);
	
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
	
	// Vaciamos la cola de eventos
	std::queue<dp::Object> empty;
	std::swap(this->evt_queue, empty);

}


void OnlineGameController::process_sync_data() {

	const Object& gamevars = this->sync_data["gamevars"];
	uint64_t tick = gamevars["tick"];
	uint64_t curr_tick = this->game->tick;
	
	if (tick > this->game->tick && !this->sync_postponed) {
		cout << "Posponiendo sync... " << (tick - this->game->tick) << endl;
		this->sync_postponed = true;
		return;
	}

	// Hacemos swap de los eventos recibidos y los locales los guardamos en cola auxiliar
	boost::json::array evt_q = this->sync_data["evt_queue"];
	uint64_t next_evt_id = this->sync_data["next_evt_id"];
	std::queue<dp::Object> aux_evt_q;
	for (auto& evt: evt_q) {
		//cout << "ESTABLECIENDO: " << evt << endl;
		aux_evt_q.push(evt);
	}
	std::swap(this->evt_queue, aux_evt_q);
	// Eliminamos los eventos residuales de la cola auxiliar
	while (aux_evt_q.size() > 0) {
		uint64_t front_evt_id = aux_evt_q.front()["evt_id"];
		if (front_evt_id < next_evt_id) {
			//cout << "ELIMINANDO: " << aux_evt_q.front() << endl;
			aux_evt_q.pop();
		} else {
			break;
		}
	}
	// Añadimos los eventos de la cola auxiliar (si quedase alguno) a la cola real
	while (aux_evt_q.size() > 0) {
		//cout << "RECOMPONIENDO: " << aux_evt_q.front() << endl;
		this->evt_queue.push(aux_evt_q.front());
		aux_evt_q.pop();
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
	this->sync_postponed = false;

	while (this->game->tick < curr_tick) {
		cout << "NOTICE: FLUSHIN TICK! (" << this->game->tick << ")" << endl;
		this->process_events();
		this->game->process_tick();
	}

	if (this->game->tick != curr_tick) {
		cerr << "WARN: ticks dont match: " << this->game->tick << " != " << curr_tick << endl;
	}

}


void OnlineGameController::process_events() {

	while (this->evt_queue.size() > 0) {

		auto& evt = this->evt_queue.front();
		uint64_t evt_tick = evt["tick"];

		if (evt_tick == this->game->tick) {

			this->process_event(evt);
			this->evt_queue.pop();

		} else if (evt_tick < this->game->tick) {

			cerr << "DESYNC! evt_tick: " << evt_tick << " | game_tick: " << this->game->tick << endl;
			//cerr << "EVT: " << evt << endl;
			this->evt_queue.pop();
		
			throw std::runtime_error("Evento anterior no procesado");

		} else {
			
			break;

		}

	}

}

void OnlineGameController::on_tick() {

	std::lock_guard<std::mutex> guard(this->evt_queue_mutex);

	if (!this->sync_data.json().empty()) {
		this->process_sync_data();
	}

	this->process_events();

}

} // namespace dp::client
