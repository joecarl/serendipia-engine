#include <dp/client/ui/textinput.h>
#include <dp/client/mediatools.h>
#include <dp/client/baseclient.h>

#include <allegro5/allegro_primitives.h>

namespace dp::client::ui {

TextInput::TextInput(BaseClient* _engine) :
	Input(_engine)
{
	this->type = INPUT_TYPE_TEXT;
	this->value_type = INPUT_VALUE_TYPE_STRING;
	//font = fuente;//al_load_ttf_font("font.ttf", 2*9, 0);
	reset();

}



void TextInput::reset() {

	edittext = "";
	iter = edittext.begin();
	caret = 0;
	caret_time = 0;
	insert = true;

}

void TextInput::process_key(wchar_t ASCII, int control_key) {

	/*
	if (ASCII >= 32 && ASCII <= 126) {
		
		// add the new char, inserting or replacing as need be
		if (insert || iter == edittext.end())
			iter = edittext.insert(iter, ASCII);
		else
			edittext.replace(caret, 1, 1, ASCII);
		
		// increment both the caret and the iterator
		caret++;
		iter++;

	}
	*/
	char ascii = 0;

	switch (control_key) {
		case ALLEGRO_KEY_FULLSTOP:
			ascii = '.';
			break;
		case ALLEGRO_KEY_SPACE:
			ascii = ' ';
			break;
	}
	
	if (control_key >= ALLEGRO_KEY_A && control_key <= ALLEGRO_KEY_Z) {
		ascii = control_key - ALLEGRO_KEY_A + 'A';
	}
	if (control_key >= ALLEGRO_KEY_0 && control_key <= ALLEGRO_KEY_9) {
		ascii = control_key - ALLEGRO_KEY_0 + '0';
	}

	bool processed = true;

	if (ascii >= 32 && ascii <= 126) {
		
		// add the new char, inserting or replacing as need be
		if (insert || iter == edittext.end()) {
			if (max_chars == 0 || edittext.length() < max_chars) {
				iter = edittext.insert(iter, ascii);
				caret++;
				iter++;
			}
		} else {
			edittext.replace(caret, 1, 1, ascii);
			caret++;
			iter++;
		}
		
	}

	// some other, "special" key was pressed; handle it here
	else switch (control_key) {

		case ALLEGRO_KEY_DELETE://delete
			if (iter != edittext.end()) iter = edittext.erase(iter);
			break;
			
		case ALLEGRO_KEY_BACKSPACE://backspace
			if (iter != edittext.begin())
			{
				caret--;
				iter--;
				iter = edittext.erase(iter);
			}
			break;
			
		case ALLEGRO_KEY_RIGHT:
			if (iter != edittext.end())   caret++, iter++;
			break;
			
		case ALLEGRO_KEY_LEFT:
			if (iter != edittext.begin()) caret--, iter--;
			break;
			
		case ALLEGRO_KEY_INSERT:
			insert = !insert;
			break;
			
		default:
			processed = false;
			break;
	}

	if (processed) {
		caret_time = 0;
	}

}

void TextInput::draw(float x, float y) {

	ALLEGRO_FONT* font = this->engine->get_font();

	al_draw_text(font, WHITE, x, y, ALLEGRO_ALIGN_LEFT, edittext.c_str());

	if (!this->is_focused()) {
		return;
	}

	if (caret_time < 30) {

		char text_caret[100]; 
		strcpy(text_caret, edittext.c_str());
		text_caret[caret] = '\0';
		int length = al_get_text_width(font, text_caret);
		al_draw_line(
			x + length + 2,
			y,
			x + length + 2,
			y + 2 * 6,
			WHITE, 
			1
		);
	}

	caret_time++;

	if (caret_time > 60) {
		caret_time = 0;
	}

}


std::string TextInput::get_value() {

	return edittext;

}


void TextInput::set_from_json_value(const boost::json::value& val) {
	
	if (val.is_string()) {
		this->reset();
		this->edittext = val.get_string().c_str();
		this->iter = edittext.begin();
	}

}


boost::json::value TextInput::get_json_value() {

	return boost::json::value(this->get_value());

}


bool TextInput::is_valid() {

	size_t len = edittext.length();

	const bool min_chars_valid = len >= this->min_chars;
	const bool max_chars_valid = this->max_chars == 0 || len <= this->max_chars;
	
	return min_chars_valid && max_chars_valid;

}


std::string TextInput::get_validation_msg() {

	size_t len = edittext.length();
	if (len < this->min_chars) {
		return "At least " + std::to_string(this->min_chars) + " characters"; 
	}

	if (len > this->max_chars) {
		return "Max: " + std::to_string(this->max_chars) + " characters"; 
	}

	return "";
}

} // namespace dp::client::ui
