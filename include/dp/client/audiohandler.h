#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <unordered_map>
#include <vector>
#include <string>

const int Do = 1, DoSos = 2, Re = 3, ReSos = 4, Mi = 5, Fa = 6, FaSos = 7,
	  Sol = 8, SolSos = 9, La = 10, LaSos = 11, Si = 12;


namespace dp::client {
	class BaseClient;
}

namespace dp::client {

struct AudioNote {

	/**
	 * The note C = 1; C# = 2; ... B = 12. Use predefined constants
	 */
	int note;

	/**
	 * The note duration when playing (seconds)
	 */
	double duration;

	/**
	 * The octave where the note is played. Default is 4 
	 */
	int octave = 4;

};

class AudioHandler {

	struct ActiveSampleData {

		/**
		 * The second from which the sample can be pruned
		 */
		double finish_at;

		/**
		 * The sample to be destroyed
		 */
		ALLEGRO_SAMPLE* beep;

		/**
		 * The data buffer to be freed
		 */
		uint16_t* buff;

		/**
		 * The sample id to be stopped
		 */
		ALLEGRO_SAMPLE_ID sampleid;

	};

	
	struct AudioNoteRes {

		/**
		 * The calculated frequency
		*/
		float freq;

		/**
		 * The samples needed
		 */
		uint64_t samples;
		
	};

	struct AudioSampleRes {

		std::vector<AudioNoteRes> notes;

		float total_duration = 0;

		uint64_t total_samples = 0;

	};

	/**
	 * The samples which are currently playing
	 */
	std::unordered_map<size_t, ActiveSampleData> active_samples;

	size_t next_audio_id = 0;

	BaseClient* client;

	void play_sample(const AudioSampleRes& res, double volume, ALLEGRO_PLAYMODE mode);

public:

	AudioHandler(BaseClient* _client) : client(_client) { }

	~AudioHandler() { this->prune(true); }

	void create_and_play_sample(const std::vector<AudioNote>& notes, double volume = 1.0, ALLEGRO_PLAYMODE mode = ALLEGRO_PLAYMODE_ONCE);

	/**
	 * This function should be called periodically in order to free resources
	 */
	void prune(bool hard = false);

};

} // namespace dp::client

#endif
