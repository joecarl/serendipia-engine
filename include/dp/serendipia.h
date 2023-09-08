#ifndef SERENDIPIA_H
#define SERENDIPIA_H

#include <string>
#include <cstdint>

namespace dp {

typedef struct {

	const std::string version;

	const std::string name;

	const std::string pkgname;
	
} AppInfo;


class BaseGame {

public:

	bool finished = false;

	unsigned int tick = 0;

	bool is_finished() { return this->finished; }

	virtual uint_fast32_t get_rnd_seed() { return 0; }

	virtual void process_tick() = 0;

	virtual void set_player_control_state(int player_idx, int control, bool new_state) = 0;

	virtual ~BaseGame() {}

};

} // namespace dp

#endif
