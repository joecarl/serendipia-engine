#ifndef INPUT_H
#define INPUT_H

#include <boost/json.hpp>
#include <string>

namespace dp::client {
	class BaseClient;
}

namespace dp::client::ui {

enum InputType {
	INPUT_TYPE_TEXT,
	INPUT_TYPE_SELECT,
	INPUT_TYPE_TOGGLE,
	INPUT_TYPE_MAX
};

enum InputValueType {
	INPUT_VALUE_TYPE_STRING,
	INPUT_VALUE_TYPE_BOOL,
	INPUT_VALUE_TYPE_DOUBLE,
	INPUT_VALUE_TYPE_INTEGER,
	INPUT_VALUE_TYPE_MAX
};

class Input {

protected:
	
	/**
	 * The font used to draw the text
	 */
	BaseClient* const engine;

	InputType type;

	InputValueType value_type;

public:

	std::string label;

	//double x;

	//double y;

	Input(BaseClient* _engine);

	virtual void draw(float x, float y) = 0;

	void focus();

	void blur();

	bool is_focused();

	virtual void process_key(wchar_t ASCII, int control_key) { }
	
	virtual bool is_valid() = 0;

	virtual std::string get_validation_msg() = 0;
	
	virtual void set_from_json_value(boost::json::value& val) = 0;

	virtual boost::json::value get_json_value() = 0;

	InputType get_type() { return this->type; }

	InputValueType get_value_type() { return this->value_type; }

};

} // namespace dp::client::ui

#endif
