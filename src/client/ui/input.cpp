#include <dp/client/ui/input.h>
#include <dp/client/baseclient.h>

namespace dp::client::ui {

Input::Input(BaseClient* _engine) : engine(_engine) { 

}

void Input::focus() {
	
	this->engine->set_active_input(this);
	
}


bool Input::is_focused() {

	return this->engine->get_active_input() == this;

}


void Input::blur() {

	if (this->is_focused()) {
		this->engine->set_active_input(nullptr);
	} 

}

} // namespace dp::client::ui