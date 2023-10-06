#ifndef BASECLIENT_H
#define BASECLIENT_H

#include <dp/serendipia.h>
#include <dp/client/connection.h>
#include <dp/client/netgroupshandler.h>
#include <dp/client/allegrohandler.h>
#include <dp/client/stage.h>
#include <dp/client/ui/touchkeys.h>
#include <dp/client/ui/textinput.h>
#include <dp/client/ui/selectinput.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <boost/json.hpp>

#define TICKS_PER_SECOND 60.0

namespace dp::client {

class BaseClient {

	using TouchKeys = ui::TouchKeys;
	using Input = ui::Input;
	using TextInput = ui::TextInput;
	using SelectInput = ui::SelectInput;

	double old_time = 0;

	unsigned int frames_done = 0;

	bool must_run_on_enter_stage = false;

	std::string resources_dir;

	std::string custom_cfg_filepath;

	AllegroHandler allegro_hnd;

	TouchKeys touch_keys;

	TouchKeys kb_touch_keys;

	TouchKeys* active_touch_keys;

	std::vector<std::unique_ptr<TextInput>> text_inputs;

	std::vector<std::unique_ptr<SelectInput>> select_inputs;

	// TODO: ver si es posible algo así:
	//std::vector<std::unique_ptr<Input>> inputs;

	Input* active_input = nullptr;

	Connection connection;

	//NetGroupsHandler groups_handler;

	ALLEGRO_FONT* font;

	ALLEGRO_BITMAP* kb_icon;

	boost::json::object cfg;

	boost::json::object default_cfg;

	float fps = 0;

	float scale = 1.0;

	unsigned int active_stage_id = 0;

	std::unordered_map<uint16_t, Stage*> stages;

	bool keys[ALLEGRO_KEY_MAX];

	uint16_t res_x = 320/*DEF_W*/, res_y = 200/*DEF_H*/;

	void calc_fps();

	void run_tick();

	void on_event(ALLEGRO_EVENT event);

	bool process_touch_keys(ALLEGRO_EVENT& event);

	void draw();

	void set_active_touch_keys(TouchKeys& tkeys);

public:

	virtual Stage* create_stage(uint16_t id) = 0;

	std::string debug_txt = "";

	bool finish = false;

	const AppInfo app_info;

	BaseClient(const AppInfo& app_info, const std::string& res_dir);

	void set_stage(unsigned int stage_id);

	void run();

	void set_cfg_param(const std::string& key, const boost::json::value& val);
	
	/**
	 * Obtain a valid writeable directory to save data
	 */
	const std::string get_storage_dir();

	ALLEGRO_BITMAP* load_bitmap_resource(const std::string& filename);

	TextInput* create_text_input();

	SelectInput* create_select_input();

	/**
	 * A select input with predefined options (YES/NO) and boolean value type
	 */
	SelectInput* create_boolean_input();
	
	void set_active_input(Input* input);

	Input* get_active_input();

	TouchKeys& get_touch_keys() { return this->touch_keys; }

	Connection& get_io_client() { return this->connection; }

	boost::json::object& get_cfg() { return this->cfg; }

	boost::json::object& get_default_cfg() { return this->default_cfg; }

	AllegroHandler& get_allegro_hnd() { return this->allegro_hnd; }

	float get_scale() { return this->scale; }

	float get_fps() { return this->fps; }

	uint16_t get_res_x() { return this->res_x; }

	uint16_t get_res_y() { return this->res_y; }

	ALLEGRO_FONT* get_font() { return this->font; }

	//NetGroupsHandler& get_groups_handler() { return this->groups_handler; }

	bool get_key(uint16_t kcode) { return kcode < ALLEGRO_KEY_MAX ? this->keys[kcode]: false; }

};

} // namespace dp::client

#endif
