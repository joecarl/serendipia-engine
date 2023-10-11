#include <dp/client/audiohandler.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <boost/thread.hpp>
#include <cmath>
#include <string>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#define FREC_MUESTREO 48000

namespace dp::client {

void AudioHandler::create_and_play_sample(const std::vector<AudioNote>& notes, double volume, ALLEGRO_PLAYMODE mode) {

	if (volume <= 0) return;

	AudioSampleRes res;
	
	for (auto& note: notes) {
		
		float freq = 440.0 * pow(2.0, (float) (note.octave - 3.0 + (note.note - 10.0) / 12.0));
		// Obtenemos un numero de ciclos y lo guardamos como valor entero
		int ciclos = freq * note.duration;
		float real_duration = float(ciclos) / freq;

		uint64_t samples = FREC_MUESTREO * real_duration / 1000.0;

		res.notes.push_back({
			.freq = freq,
			.samples = samples
		});

		res.total_duration += real_duration;
		res.total_samples += samples;

	}

	this->play_sample(res, volume, mode);

}


void AudioHandler::play_sample(const AudioSampleRes& res, double volume, ALLEGRO_PLAYMODE mode) {

	uint16_t* buff = new uint16_t[res.total_samples];
	uint16_t* buff_ptr = buff;
	for (auto& note_res: res.notes) {
		for (uint64_t i = 0; i < note_res.samples; i++) {
			*buff_ptr++ = 30000 * sin(note_res.freq * 2 * 3.14 * i / FREC_MUESTREO) * sin(3.14 * i / note_res.samples);
		}
	}

	if (volume > 1.0) volume = 1.0;

	ALLEGRO_SAMPLE *beep = nullptr;
	beep = al_create_sample(buff, res.total_samples, FREC_MUESTREO, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1, false);
   
	if (beep == nullptr) {
		std::cerr << "Error creating sample" << std::endl;
		return;
	}

	double duration = 0.1 + res.total_duration / 1000.0; // Add 0.1s guard so audio is not unexpectedly stopped
	std::cout << "Playing sample for " << duration << "seconds" << std::endl;
	ALLEGRO_SAMPLE_ID sampleid;
	bool played = al_play_sample(beep, volume, 0, 1.0, mode, &sampleid);

	if (!played) {
		std::cerr << "Error playing sample. Not enough reserved samples?" << std::endl;
		return;
	}

	this->active_samples[this->next_audio_id++] = {
		.finish_at = al_get_time() + duration,
		.beep = beep,
		.buff = buff,
		.sampleid = sampleid,
	};

}


void AudioHandler::prune(bool hard) {

	auto time = al_get_time();
	std::vector<size_t> to_erase;

	for (auto& active_sample_it: this->active_samples) {

		auto& active_sample = active_sample_it.second;
		if (!hard && time < active_sample.finish_at) {
			continue;
		}

		al_stop_sample(&active_sample.sampleid);
		al_destroy_sample(active_sample.beep);
		delete[] active_sample.buff;
		std::cout << "Sample destroyed" << std::endl;
		to_erase.push_back(active_sample_it.first);

	}

	for (auto& k: to_erase) {
		this->active_samples.erase(k);
	}

}

/*
#define o 3
void play_exorcista() {

	play_sound(Mi, 100, o);
	play_sound(La, 200, o);
	play_sound(Mi, 100, o);
	play_sound(Si, 200, o);
	play_sound(Mi, 100, o);
	play_sound(Sol, 200, o);
	play_sound(La, 200, o);
	play_sound(Mi, 100, o);
	play_sound(Do, 200, o + 1);
	play_sound(Mi, 100, o);
	play_sound(Re, 200, o + 1);
	play_sound(Mi, 100, o);
	play_sound(Si, 200, o);
	play_sound(Do, 200, o + 1);
	play_sound(Mi, 100, o);
	play_sound(Si, 200, o);
	play_audio(1, ALLEGRO_PLAYMODE_LOOP);

}
*/

/*
void ShowKeyBoardMatrix() {
	int i,j;
	acquire_screen();
	clear_keybuf();
	for (i=0;i<3;i++) {
		for (j=0;j<10;j++) {
			char a[2];
			a[1]='\0';
			if (key[KEY_A+i*10+j])a[0]='1';
			else a[0]='0';

			textout_ex(screen,font,a,(j+1)*10,(i+1)*10,makecol(255,255,255),makecol(0,0,0));
		}
	}
	release_screen();
}
*/

} // namespace dp::client


