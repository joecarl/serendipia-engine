#ifndef ONLINEGAMECONTROLLER_H
#define ONLINEGAMECONTROLLER_H

#include <dp/serendipia.h>
#include <dp/object.h>
#include <queue>
#include <mutex>

namespace dp::client {

/**
 * This class controls the online playmode. It allows the proper handling of
 * received events and processes synchronization with the server when needed.
 */
class OnlineGameController {

	BaseGame *game;

	/**
	 * Objet used to store the sync data received from server. It will be 
	 * flushed after sync.
	 */
	Object sync_data;

	/**
	 * Determines if sync was postponed or not. If sync data tick is ahead
	 * from game->tick then synchronization must be postponed at least 1 tick.
	 */
	bool sync_postponed;

	/**
	 * Queue where received events are stored to be processed on the 
	 * corresponding tick.
	 */
	std::queue<Object> evt_queue;

	std::mutex evt_queue_mutex;

	void process_event(const Object& evt);

	void process_events();
	
	void process_sync_data();

public:

	/**
	 * Custom handler for processing sync data. It's the game designer 
	 * responsibility to define this function.
	 */
	std::function<void(BaseGame*, const Object&)> sync_game = nullptr;

	/**
	 * Push a game event to the queue. This must be called from the networking
	 * thread.
	 */
	void push_event(const Object& evt);

	/**
	 * Setup the game to control.
	 */
	void setup(BaseGame *game);

	/**
	 * Yield 1 tick. This should be called before processing the game tick.
	 */
	void on_tick();

};

} // namespace dp::client

#endif