#ifndef ALLEGROHANDLER_H
#define ALLEGROHANDLER_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#define ALPHA_COLOR al_map_rgb(255, 0, 255)

#define WHITE al_map_rgb(255, 255, 255)
#define RED al_map_rgb(255, 0, 0)

#define CGA_PINK al_map_rgb(255, 85, 255)
#define CGA_BLUE al_map_rgb(85, 255, 255)

namespace dp::client {

class BaseClient;

struct Point {
	int x, y;
};

class AllegroHandler {
	
	BaseClient *engine;

	ALLEGRO_TIMER* timer;

	ALLEGRO_DISPLAY *display = NULL;

	int screen_width, screen_height;

	/**
	 * Main buffer where all non-UI components will render
	 */
	ALLEGRO_BITMAP *buffer;
	
	ALLEGRO_BITMAP *auxb;

	/**
	 * Secondary buffer mainly used to render UI components
	 */
	ALLEGRO_BITMAP *sec_buffer;

	ALLEGRO_EVENT_QUEUE *event_queue;

	/**
	 * How much the buffer should be scaled
	 */
	int scale_w, scale_h, scale_x, scale_y;

	int window_width, window_height;

	float scaled;

	void cleanup();

public:

	AllegroHandler(BaseClient *game_engine);

	~AllegroHandler();

	void initialize_resources();

	void extract_assets(const std::string& path);

	void create_components();

	Point get_mapped_coordinates(int real_x, int real_y);

	void fit_display();

	int get_window_width();

	int get_window_height();

	void start_drawing();

	void prepare_main_surface();

	void draw_main_surface();

	void prepare_sec_surface();

	void draw_sec_surface();

	void finish_drawing();

	float get_scaled();

	ALLEGRO_EVENT_QUEUE* get_event_queue() { return this->event_queue; }

};

ALLEGRO_BITMAP* load_bitmap(const std::string& filename);

} // namespace dp::client

#endif