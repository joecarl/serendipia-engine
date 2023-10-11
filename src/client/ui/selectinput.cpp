#include <dp/client/ui/selectinput.h>
#include <dp/client/baseclient.h>

#include <allegro5/allegro_primitives.h>
#include <iostream>

namespace dp::client::ui {

SelectInput::SelectInput(BaseClient* _engine) :
	Input(_engine)
{
	this->type = INPUT_TYPE_SELECT;
	this->value_type = INPUT_VALUE_TYPE_STRING;
	reset();

}


void SelectInput::set_options(std::vector<SelectOption>& opts) {

	this->options = opts;
	this->reset();

}

void SelectInput::set_value_type(InputValueType vtype) {
	this->value_type = vtype;
}



void SelectInput::reset() {

	this->iter = this->options.begin();

}


void SelectInput::process_key(wchar_t ASCII, int control_key) {

	if (this->options.size() == 0) {
		return;
	}

	auto last_item = this->options.end() - 1;
	bool loop = true;

	if (control_key == ALLEGRO_KEY_RIGHT) {
		if (iter < last_item) {
			iter++;
		} else if (loop) {
			iter = this->options.begin();
		}
	} else if (control_key == ALLEGRO_KEY_LEFT) {
		if (iter > this->options.begin()) {
			iter--;
		} else if (loop) {
			iter = last_item;
		}
	}
	
}


void SelectInput::draw(float x, float y) {

	ALLEGRO_FONT* font = this->engine->get_font();

	al_draw_text(font, WHITE, x, y, ALLEGRO_ALIGN_LEFT, iter->label.c_str());

	if (!this->is_focused()) {
		return;
	}
	// TODO: draw arrows
}


std::string SelectInput::get_value() {

	return iter->value;

}


bool SelectInput::get_value_as_bool() {
	
	return this->get_value() == "true";

}


void SelectInput::set_from_json_value(const boost::json::value& val) {

	std::string v;
	if (val.is_string()) {
		v = val.get_string().c_str();
	} else if (val.is_bool()) {
		v = val.get_bool() ? "true" : "false";
	} else {
		return;
	}

	auto it = options.begin();
	while (it != options.end() && it->value != v) {
		it++;
	}
	iter = it;
	
}


boost::json::value SelectInput::get_json_value() {

	if (this->value_type == INPUT_VALUE_TYPE_BOOL) {
		return boost::json::value(this->get_value_as_bool());
	}
	// TODO: implement more conversions
	return boost::json::value(this->get_value());

}


bool SelectInput::is_valid() {
	
	return iter >= options.begin() && iter < options.end();

}


std::string SelectInput::get_validation_msg() {

	return "";

}

} // namespace dp::client::ui
