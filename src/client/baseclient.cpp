#include <dp/client/baseclient.h>
#include <dp/client/mediatools.h>
#include <dp/utils.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include <iostream>
#include <filesystem>

using std::cout;
using std::endl;
using dp::client::ui::TextInput;
using dp::client::ui::TouchKeys;
using dp::client::ui::TouchKeysCell;

namespace dp::client {

static void setup_touch_kb(TouchKeys& tk) {

	tk.add_button(ALLEGRO_KEY_1, "1");
	tk.add_button(ALLEGRO_KEY_2, "2");
	tk.add_button(ALLEGRO_KEY_3, "3");
	tk.add_button(ALLEGRO_KEY_4, "4");
	tk.add_button(ALLEGRO_KEY_5, "5");
	tk.add_button(ALLEGRO_KEY_6, "6");
	tk.add_button(ALLEGRO_KEY_7, "7");
	tk.add_button(ALLEGRO_KEY_8, "8");
	tk.add_button(ALLEGRO_KEY_9, "9");
	tk.add_button(ALLEGRO_KEY_0, "0");

	tk.add_button(ALLEGRO_KEY_Q, "Q");
	tk.add_button(ALLEGRO_KEY_W, "W");
	tk.add_button(ALLEGRO_KEY_E, "E");
	tk.add_button(ALLEGRO_KEY_R, "R");
	tk.add_button(ALLEGRO_KEY_T, "T");
	tk.add_button(ALLEGRO_KEY_Y, "Y");
	tk.add_button(ALLEGRO_KEY_U, "U");
	tk.add_button(ALLEGRO_KEY_I, "I");
	tk.add_button(ALLEGRO_KEY_O, "O");
	tk.add_button(ALLEGRO_KEY_P, "P");

	tk.add_button(ALLEGRO_KEY_A, "A");
	tk.add_button(ALLEGRO_KEY_S, "S");
	tk.add_button(ALLEGRO_KEY_D, "D");
	tk.add_button(ALLEGRO_KEY_F, "F");
	tk.add_button(ALLEGRO_KEY_G, "G");
	tk.add_button(ALLEGRO_KEY_H, "H");
	tk.add_button(ALLEGRO_KEY_J, "J");
	tk.add_button(ALLEGRO_KEY_K, "K");
	tk.add_button(ALLEGRO_KEY_L, "L");

	tk.add_button(ALLEGRO_KEY_Z, "Z");
	tk.add_button(ALLEGRO_KEY_X, "X");
	tk.add_button(ALLEGRO_KEY_C, "C");
	tk.add_button(ALLEGRO_KEY_V, "V");
	tk.add_button(ALLEGRO_KEY_B, "B");
	tk.add_button(ALLEGRO_KEY_N, "N");
	tk.add_button(ALLEGRO_KEY_M, "M");
	tk.add_button(ALLEGRO_KEY_BACKSPACE, "<<");
	
	tk.add_button(ALLEGRO_KEY_LEFT, "<");
	tk.add_button(ALLEGRO_KEY_RIGHT, ">");
	tk.add_button(ALLEGRO_KEY_SPACE, " ");
	tk.add_button(ALLEGRO_KEY_FULLSTOP, ".");
	tk.add_button(ALLEGRO_KEY_ENTER, "Ok");

	TouchKeysCell c = {
		.width = 1,
		.flex = true,
	};

	tk.layout_buttons({
		{ .height = 7,  .flex = true,  .cells = {} },
		{ .height = 25, .min_flex_height = 1, .flex = false, .cells = { c, c, c, c, c, c, c, c, c, c } },
		{ .height = 25, .min_flex_height = 1, .flex = false, .cells = { c, c, c, c, c, c, c, c, c, c } },
		{ .height = 25, .min_flex_height = 1, .flex = false, .cells = { c, c, c, c, c, c, c, c, c } },
		{ .height = 25, .min_flex_height = 1, .flex = false, .cells = { c, c, c, c, c, c, c, c } },
		{ .height = 25, .min_flex_height = 1, .flex = false, .cells = { c, c, { .width = 3, .flex = true }, c, c } },
	});

}


BaseClient::BaseClient(const AppInfo& _app_info, const std::string& res_dir) : 
	resources_dir(res_dir),
	//custom_cfg_filepath(this->get_storage_dir() + "/cfg.json"),
	allegro_hnd(this),
	touch_keys(this),
	kb_touch_keys(this),
	active_touch_keys(&touch_keys),
	connection(this),
	app_info(_app_info)
{

	cout << "Initializing resources..." << endl;
	this->allegro_hnd.initialize_resources();
#ifdef __ANDROID__
	this->allegro_hnd.extract_assets(res_dir);
	resources_dir = this->get_storage_dir() + "/" + res_dir;
#endif

	this->custom_cfg_filepath = this->get_storage_dir() + "/cfg.json";
	const std::string default_cfg_filepath = resources_dir + "/defaultCfg.json";
	
	if (file_exists(custom_cfg_filepath)) {

		boost::json::value cfg_v = boost::json::parse(file_get_contents(custom_cfg_filepath));

		if (cfg_v.is_object()) {
			this->cfg = cfg_v.get_object();
		}

	}
	
	if (file_exists(default_cfg_filepath)) {

		const boost::json::value cfg_v = boost::json::parse(file_get_contents(default_cfg_filepath));

		if (cfg_v.is_object()) {
			this->default_cfg = cfg_v.get_object();
		}

	} else {

		this->default_cfg["windowed"] = false;

	}

	for (auto& item: this->default_cfg) {
		if (!this->cfg.contains(item.key())) {
			this->cfg[item.key()] = item.value();
		}
	}

	cout << "CFG: " << this->cfg << endl;

	//this->allegro_hnd = new AllegroHandler(this); 

	cout << "Creating components..." << endl;
	this->allegro_hnd.create_components();

	for (unsigned int i = 0; i < sizeof(keys); i++) {
		keys[i] = false;
	}

	std::string font_dir = res_dir + "/font.ttf";

	font = al_load_ttf_font(font_dir.c_str(), scale * 10, ALLEGRO_TTF_MONOCHROME);

	kb_icon = load_bitmap(res_dir + "/keyboard.png");

	setup_touch_kb(kb_touch_keys);

}


void BaseClient::set_cfg_param(const std::string& key, const boost::json::value& val) {

	this->cfg[key] = val;
	file_put_contents(this->custom_cfg_filepath, boost::json::serialize(this->cfg));

}


const std::string BaseClient::get_storage_dir() {

#ifdef __ANDROID__

	return "/data/data/" + this->app_info.pkgname + "/files";

#else

	std::string path = "./data"; // TODO: get real writable path
	std::filesystem::create_directories(path);
	return path;

#endif

}


bool BaseClient::process_touch_keys(ALLEGRO_EVENT& event) {

	ALLEGRO_EVENT prev_event = event;
	this->active_touch_keys->redefine_touch_event(event);

	if (prev_event.type == ALLEGRO_EVENT_TOUCH_BEGIN && event.type == ALLEGRO_EVENT_KEY_DOWN) {

		if (
			this->active_touch_keys == &(this->kb_touch_keys) &&
			event.keyboard.keycode == ALLEGRO_KEY_ENTER
		) {
			// if touch kb is present and enter key was touched
			// capture this event to revert to previous touch keys
			// and stop propagation
			this->set_active_touch_keys(this->touch_keys);
			return false;

		} else {
			// in any other situation also trigger as char event
			ALLEGRO_EVENT char_event = event;
			char_event.type = ALLEGRO_EVENT_KEY_CHAR;
			this->on_event(char_event);
		}

	}

	return true;

}


void BaseClient::run() {

	cout << "Main loop starts" << endl;

	ALLEGRO_EVENT event;

	bool drawing_halted = false;
	
	do {

		auto evt_queue = this->allegro_hnd.get_event_queue();

		al_wait_for_event(evt_queue, &event);

		if (
			event.type == ALLEGRO_EVENT_KEY_CHAR || 
			event.type == ALLEGRO_EVENT_KEY_DOWN ||
			event.type == ALLEGRO_EVENT_KEY_UP ||
			event.type == ALLEGRO_EVENT_TOUCH_BEGIN ||
			event.type == ALLEGRO_EVENT_TOUCH_END ||
			event.type == ALLEGRO_EVENT_TOUCH_MOVE
		) {
			
			if (this->active_touch_keys != nullptr) {

				const bool propagate = this->process_touch_keys(event);
				if (!propagate) {
					continue;
				}

			}

			if (event.keyboard.keycode == ALLEGRO_KEY_BACK) {
				//Bind back button to Escape key
				event.keyboard.keycode = ALLEGRO_KEY_ESCAPE;
			}

			this->on_event(event);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
			drawing_halted = true;
			al_acknowledge_drawing_halt(event.display.source);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) {
			drawing_halted = false;
			al_acknowledge_drawing_resume(event.display.source);
		}

		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			al_acknowledge_resize(event.display.source);
			this->allegro_hnd.fit_display();
			this->active_touch_keys->re_arrange();
		}

		else if (event.type == ALLEGRO_EVENT_TIMER) {

			this->run_tick();

			if (!drawing_halted && al_is_event_queue_empty(evt_queue)) {

				this->draw();

			}

		}

	} while (!this->finish);

	cout << "Exiting..." << endl;

}


void BaseClient::calc_fps() {

	double game_time = al_get_time();

	if (game_time - old_time >= 1.0) {

		fps = ((float)frames_done) / (game_time - old_time);
		frames_done = 0;
		old_time = game_time;

	}

	frames_done++;

}


void BaseClient::set_stage(unsigned int stage_id) {

	if (this->stages.find(stage_id) == this->stages.end()) {
		Stage* stg = this->create_stage(stage_id);
		if (stg == nullptr) {
			std::cerr << " > unable to create stage: " << stage_id << std::endl;
			return;
		}
		this->stages[stage_id] = stg;
	}
	
	this->active_stage_id = stage_id;
	this->must_run_on_enter_stage = true;
	
	this->set_active_input(nullptr);

	std::cout << " > set_stage: " << this->active_stage_id << std::endl;

}


void BaseClient::run_tick() {

	//std::cout << "run_tick... ";

	auto active_stage = (Stage*) this->stages[this->active_stage_id];

	if (this->must_run_on_enter_stage) {
		active_stage->on_enter_stage();
		this->must_run_on_enter_stage = false;
	}

	active_stage->on_tick();

	//std::cout << "done!" << std::endl;

}


void BaseClient::draw() {

	this->allegro_hnd.start_drawing();
	//std::cout << "drawing... ";
	this->allegro_hnd.prepare_main_surface();

	this->calc_fps();

	auto active_stage = (Stage*) this->stages[this->active_stage_id];

	active_stage->draw();

	//al_draw_text(font, al_map_rgb(255, 255, 0), 5, 20, ALLEGRO_ALIGN_LEFT, this->debug_txt.c_str());

	this->allegro_hnd.draw_main_surface();
	
	this->allegro_hnd.prepare_sec_surface();
	
	
	#ifdef ALLEGRO_ANDROID
	//this->kb_touch_keys.draw();
	if (this->active_touch_keys != nullptr) {
		this->active_touch_keys->draw();
	}
	
	if (this->get_active_input()) {
		ALLEGRO_COLOR btn_bgcolor = al_map_rgb(150, 200, 150);
		if (this->active_touch_keys != &(this->kb_touch_keys)) {
			btn_bgcolor = al_map_rgb(120, 140, 120);
		}
		al_draw_filled_rounded_rectangle(15, -10, 58, 24, 6, 6, btn_bgcolor);
		al_draw_bitmap(kb_icon, 20, 6, 0);
	}
	#endif
	
	this->allegro_hnd.draw_sec_surface();

	this->allegro_hnd.finish_drawing();

	//std::cout << "done!" << std::endl;

}


void BaseClient::set_active_touch_keys(TouchKeys& tkeys) {

	this->active_touch_keys = &tkeys;
	this->active_touch_keys->re_arrange();

}


TextInput* BaseClient::create_text_input() {

	auto inp = std::make_unique<TextInput>(this);
	TextInput* inp_ptr = inp.get();

	this->text_inputs.push_back(std::move(inp));
	
	return inp_ptr;
	/*
	// the code below causes segfaults because push_back may resize the vector 
	// and addresses change	so returning the address was just returning a 
	// temporary address which leaded to segfaults
	this->text_inputs.push_back(TextInput(this));
	TextInput& inp = this->text_inputs.back();
	return &inp;
	*/

}

void BaseClient::set_active_input(TextInput* inp) {
	this->active_input = inp;
}

TextInput* BaseClient::get_active_input() {
	return this->active_input;
}


void BaseClient::on_event(ALLEGRO_EVENT event) {

	auto active_stage = (Stage*) this->stages[this->active_stage_id];

	if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
		
		keys[event.keyboard.keycode] = true;

	}

	else if (event.type == ALLEGRO_EVENT_KEY_UP) {
		
		keys[event.keyboard.keycode] = false;

	}

	else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {

		if (this->active_input != nullptr) {
			
			if (event.keyboard.keycode != ALLEGRO_KEY_ENTER) {

				this->active_input->process_key(event.keyboard.unichar, event.keyboard.keycode);

			}

		}

	}

	else if (
		event.type == ALLEGRO_EVENT_TOUCH_BEGIN && 
		this->get_active_input()
	) {

		float scaled = this->allegro_hnd.get_scaled();
		float tx = event.touch.x / scaled;
		float ty = event.touch.y / scaled;
		
		if (tx < 70 && ty < 30) {
			if (this->active_touch_keys != &(this->kb_touch_keys)) {
				this->set_active_touch_keys(this->kb_touch_keys);
			} else {
				this->set_active_touch_keys(this->touch_keys);
			}
		}
	}

	active_stage->on_event(event);

}

} // namespace dp::client
