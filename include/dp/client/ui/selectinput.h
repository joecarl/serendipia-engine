#ifndef SELECTINPUT_H
#define SELECTINPUT_H

#include <dp/client/ui/input.h>
#include <string>
#include <vector>

namespace dp::client { 
	class BaseClient;
}

namespace dp::client::ui {

//template <typename T>
struct SelectOption {

	std::string label;

	std::string value;

};

class SelectInput : public Input {
	
	/**
	 * String iterator
	 */
	std::vector<SelectOption>::iterator iter;

	/**
	 * The array of options
	 */
	std::vector<SelectOption> options;

public:

	SelectInput(BaseClient* _engine);

	void set_options(std::vector<SelectOption>& opts);
	
	void set_value_type(InputValueType vtype);

	void reset();

	void draw(float x, float y);

	bool is_valid();
	
	void process_key(wchar_t ASCII, int control_key);

	std::string get_validation_msg();
	
	void set_from_json_value(const boost::json::value& val);

	boost::json::value get_json_value();

	std::string get_value();

	bool get_value_as_bool();

};

} // namespace dp::client::ui

#endif