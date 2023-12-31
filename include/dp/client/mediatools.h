#ifndef MEDIATOOLS_H
#define MEDIATOOLS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <string>

#define ALPHA_COLOR al_map_rgb(255, 0, 255)

#define WHITE al_map_rgb(255, 255, 255)
#define RED al_map_rgb(255, 0, 0)

#define CGA_PINK al_map_rgb(255, 85, 255)
#define CGA_BLUE al_map_rgb(85, 255, 255)


const int Do = 1, DoSos = 2, Re = 3, ReSos = 4, Mi = 5, Fa = 6, FaSos = 7,
	  Sol = 8, SolSos = 9, La = 10, LaSos = 11, Si = 12;//
/*const Do=522, DoSos=554, Re=588, ReSos=622, Mi=660, Fa=698, FaSos=740,
	  Sol=784, SolSos=830, La=880, LaSos=932, Si=988*/

namespace dp::client {

void play_sound(int nota, float time, int octava = 4);

//void  ShowKeyBoardMatrix();

ALLEGRO_BITMAP* load_bitmap(const std::string& filename);

void play_audio(float volumen = 1.0, ALLEGRO_PLAYMODE mode = ALLEGRO_PLAYMODE_ONCE);

void play_exorcista();

} // namespace dp::client

#endif
