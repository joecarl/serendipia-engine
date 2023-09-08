#include <dp/client/baseclient.h>
#include <dp/client/mediatools.h>
#include <dp/utils.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h> /* al_android_set_apk_file_interface */
#endif

#include <iostream>

using std::cout;
using std::endl;

namespace dp::client {

AllegroHandler::AllegroHandler(BaseClient *game_engine) :
	engine(game_engine)
{

}

AllegroHandler::~AllegroHandler() {

	this->cleanup();

}

void AllegroHandler::initialize_resources() {

	cout << "Initializing addons..." << endl;
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	
	cout << "Initializing allegro..." << endl;
	if (!al_init()) {
		throw std::runtime_error("failed to initialize allegro!");
	}
	
	if (!al_install_audio()) {
		throw std::runtime_error("could not init sound!");
	}
	if (!al_reserve_samples(1)) {
		throw std::runtime_error("could not reserve samples!"); 
	}
	
	cout << "Installing keyboard..." << endl;
	if (!al_install_keyboard()) {
		throw std::runtime_error("could not init keyboard!");
	}
	
	#ifdef ALLEGRO_ANDROID
	cout << "Installing touch input..." << endl;
	if (!al_install_touch_input()) {
		throw std::runtime_error("could not init touch input!");
	}
	
	al_android_set_apk_file_interface();
	#endif

	cout << "Allegro initialized" << endl;

}

void AllegroHandler::create_components() {

	bool windowed = this->engine->get_cfg()["windowed"].as_bool();
	
	if (!windowed) {
		al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	}

	//al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);

	//ALLEGRO_DISPLAY_MODE disp_data;
	/*
	for (int i = 0; i < al_get_num_display_modes(); i++) {
		al_get_display_mode(i, &disp_data);
		cout << "Resolution: " << disp_data.width << "x" << disp_data.height << endl;
	}
	*/	
	//al_get_display_mode(al_get_num_display_modes() - 1, &disp_data);

	auto scale = this->engine->get_scale();

	this->screen_width = scale * this->engine->get_res_x();
	this->screen_height = scale * this->engine->get_res_y();

	display = al_create_display(640, 400);//disp_data.width, disp_data.height);
	if (!display) {
		throw std::runtime_error("failed to create display!");
	}

	this->buffer = al_create_bitmap(screen_width, screen_height);
	this->sec_buffer = nullptr;

	this->fit_display();

	if (!windowed) {
		al_hide_mouse_cursor(display);
	}
	
	cout << "Display created" << endl;

	//al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	this->timer = al_create_timer(1.0 / TICKS_PER_SECOND);
	this->event_queue = al_create_event_queue();

	#ifdef ALLEGRO_ANDROID
	al_register_event_source(event_queue, al_get_touch_input_event_source());
	#endif
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	al_start_timer(timer);
	
}


int AllegroHandler::get_window_width() {
	
	return al_get_display_width(display);

}

int AllegroHandler::get_window_height() {
	
	return al_get_display_height(display);
	
}



void AllegroHandler::fit_display() {

	this->window_width = al_get_display_width(display);
	this->window_height = al_get_display_height(display);

	// calculate scaling factor
	float sx = float(window_width) / screen_width;
	float sy = float(window_height) / screen_height;

	this->scaled = std::min(sx, sy);

	#ifdef __ANDROID__
	if (screen_height * scaled > 0.85 * this->window_height) {
		this->scaled *= 0.85;
	}
	#endif

	// calculate how much the buffer should be scaled
	this->scale_w = screen_width * scaled;
	this->scale_h = screen_height * scaled;
	this->scale_x = (window_width - scale_w) / 2;
	this->scale_y = (window_height - scale_h) / 2;

	if (this->sec_buffer) {
		al_destroy_bitmap(this->sec_buffer);
	}

	this->sec_buffer = al_create_bitmap(window_width / this->scaled, window_height / this->scaled);
	

}


Point AllegroHandler::get_mapped_coordinates(int real_x, int real_y) {

	return {
		(int)((real_x - scale_x) / scaled),
		(int)((real_y - scale_y) / scaled)	
	};

}

void AllegroHandler::start_drawing() {

	al_set_target_backbuffer(display);
	al_clear_to_color(al_map_rgb(0, 0, 0));

}

void AllegroHandler::prepare_main_surface() {

	al_set_target_bitmap(buffer);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
}

void AllegroHandler::draw_main_surface() {

	al_set_target_backbuffer(display);
	al_draw_scaled_bitmap(buffer, 0, 0, screen_width, screen_height, scale_x, scale_y, scale_w, scale_h, 0);

}

void AllegroHandler::prepare_sec_surface() {

	al_set_target_bitmap(sec_buffer);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
}

void AllegroHandler::draw_sec_surface() {

	al_set_target_backbuffer(display);
	al_draw_scaled_bitmap(sec_buffer, 0, 0, window_width / this->scaled, window_height / this->scaled, 0, 0, this->window_width, this->window_height, 0);

}

void AllegroHandler::finish_drawing() {

	al_wait_for_vsync();
	al_flip_display();

}

float AllegroHandler::get_scaled() {

	return this->scaled;

}

void AllegroHandler::cleanup() {

	//cout << "AllegroHandler::cleanup" << endl;
	al_uninstall_audio();
	al_uninstall_keyboard();
	#ifdef ALLEGRO_ANDROID
	al_uninstall_touch_input();
	#endif
	//al_destroy_font(this->);
	al_destroy_timer(this->timer);
	al_destroy_display(this->display);
	al_destroy_event_queue(this->event_queue);

}

} // namespace dp::client
