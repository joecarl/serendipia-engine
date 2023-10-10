#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include <dp/client/ui/input.h>

#include <allegro5/allegro_font.h>
#include <string>

namespace dp::client { 
	class BaseClient;
}

namespace dp::client::ui {

class TextInput : public Input {

	/**
	 * An empty string for editting
	 */
	std::string edittext;
	/**
	 * String iterator
	 */
	std::string::iterator iter;

	/**
	 * Tracks the text caret
	 */
	int	caret;

	/**
	 * True if should text be inserted
	 */
	bool insert;
	
	/**
	 * Caret blinking control
	 */
	int caret_time;

public:

	/**
	 * Minimun number of characters
	 */
	size_t min_chars = 0;
	
	/**
	 * Max number of characters (0: unlimited)
	 */
	size_t max_chars = 0;

	TextInput(BaseClient* _engine);

	void reset();

	void draw(float x, float y);

	bool is_valid();
	
	void process_key(wchar_t ASCII, int control_key);

	std::string get_validation_msg();
	
	void set_from_json_value(const boost::json::value& val);

	boost::json::value get_json_value();

	std::string get_value();

};

} // namespace dp::client::ui

#endif