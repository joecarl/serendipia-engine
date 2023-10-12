#ifndef ONLINEGAMECONTROLLER_H
#define ONLINEGAMECONTROLLER_H

#include <dp/serendipia.h>
#include <dp/object.h>
#include <queue>

namespace dp::client {

/**
 * This class controls the online playmode. It allows the proper handling of
 * received events and processes synchronization with the server when needed.
 */
class OnlineGameController {

	BaseGame *game;

	Object sync_data;

	std::queue<Object> evt_queue;

	void process_event(const Object& evt);
	
	void process_sync_data();

public:

	std::function<void(BaseGame*, const Object&)> sync_game = nullptr;

	void push_event(const Object& evt);

	void setup(BaseGame *game);

	void on_tick();

};

} // namespace dp::client

#endif