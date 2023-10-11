#include <dp/client/mediatools.h>

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

int last_pos = 0;
int16_t audio[FREC_MUESTREO * 5];//un audio de 5 seg máx...

void play_sound(int nota, float time, int octava) {

	float frec = 440.0 * pow(2.0, (float) (octava - 3.0 + (nota - 10.0) / 12.0));
	// Obtenemos un numero de ciclos y lo guardamos como valor entero
	int ciclos = frec * time;
	time = float(ciclos) / frec;
	unsigned int samples = FREC_MUESTREO * time / 1000.0;
	
	for (unsigned int i = 0; i < samples; i++) {

		audio[last_pos + i] = 30000 * sin(frec * 2 * 3.14 * i / FREC_MUESTREO) * sin(3.14 * i / samples);
	
	}
	
	last_pos += samples;
	
}

void play_audio(float volumen, ALLEGRO_PLAYMODE mode) {

	al_stop_samples();
	ALLEGRO_SAMPLE *beep = nullptr;

	auto buff = (uint16_t*)al_malloc(last_pos * sizeof(uint16_t));
	if (volumen > 1) volumen = 1;
	else if (volumen < 0) volumen = 0;
	for (int i = 0; i < last_pos; i++) {
		buff[i] = audio[i] * volumen;//*sin(3.140 * i / last_pos);
	}

	beep = al_create_sample(buff, last_pos, FREC_MUESTREO, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_1, true);
   
	if (beep != nullptr) {
		double duration = (double)last_pos / (double)FREC_MUESTREO;
		boost::thread([beep, mode, duration, buff] {
			ALLEGRO_SAMPLE_ID sampleid;
			al_play_sample(beep, 1.0, 0, 1.0, mode, &sampleid);
			al_rest(duration);
			al_stop_sample(&sampleid);
			al_destroy_sample(beep);
		});
	
	}

	last_pos = 0;

}

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


ALLEGRO_BITMAP* load_bitmap(const std::string& filename) {

	std::cout << "Loading bitmap: " << filename << std::endl;

	ALLEGRO_BITMAP* sprite = al_load_bitmap(filename.c_str());
	if (!sprite) {
		throw std::runtime_error("error loading bitmap");
	}
	al_convert_mask_to_alpha(sprite, ALPHA_COLOR);

	return sprite;

}

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


