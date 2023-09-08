#ifndef STAGE_H
#define STAGE_H

#include <allegro5/allegro.h>

#define TICKS_PER_SECOND 60.0

namespace dp::client {
	class BaseClient;
}

namespace dp::client {

class Stage {
	
public:

	BaseClient* engine;

	Stage(BaseClient* _engine) : engine(_engine) {};

	virtual void on_event(ALLEGRO_EVENT event) {};

	virtual void on_tick() {};

	virtual void draw() {};

	virtual void on_enter_stage() {};

};

}

#endif
