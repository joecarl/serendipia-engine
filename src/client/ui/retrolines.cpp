#include <dp/client/ui/retrolines.h>
#include <dp/client/mediatools.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <cmath>

namespace dp::client::ui {

RetroLines::RetroLines(std::vector<RetroLine>&& _lines) :
	lines(_lines)
{
	this->calc_width();
}

RetroLines::RetroLines(std::vector<std::string>&& str_lines) {

	for (auto& str_line: str_lines) {

		char prev_ch = ' ';
		uint8_t x = 0;
		uint8_t dash_width = 0;
		RetroLine retro_line;

		for (auto ch: str_line) {

			if (ch == '-') {
				dash_width++;
			} else if (ch == ' ') {
				if (prev_ch == '-') {
					const uint8_t dash_x = x - dash_width;
					retro_line.push_back({ .x = dash_x, .width = dash_width });
				}
				dash_width = 0;
			}
			prev_ch = ch;
			x++;

		}

		this->lines.push_back(move(retro_line));

	}

	this->calc_width();

}

void RetroLines::calc_width() {

	this->width = 0;

	for (auto& line: this->lines) {
		const auto& last_dash = line[line.size() - 1];
		const uint8_t line_width = last_dash.x + last_dash.width;
		if (line_width > this->width) {
			this->width = line_width;
		}
	}

	this->width *= this->mult_x;

}

void RetroLines::draw(float ox, float oy) {
	
	uint8_t line_num = 0;

	for (auto& line: this->lines) {
		for (auto& dash: line) {
			const float x = ox + dash.x * mult_x;
			const float y = oy + line_num * mult_y * 2 + sin(time + x);
			const float x2 = x + dash.width * mult_x;
			const float y2 = y + mult_y / 2.0;
			const float y3 = y + mult_y;
			//al_draw_filled_rectangle(x, y, x2, y2, al_map_rgb(180, 255, 180));
			uint32_t phase = (uint32_t)(this->time * 1.4) % 4;
			uint32_t phase2 = (uint32_t)(this->time) % 70;
			ALLEGRO_COLOR c1, c2;
			if (phase2 < 56) {
				c1 = CGA_BLUE;
				c2 = CGA_BLUE;
			} else if (phase == 0) {
				c1 = CGA_PINK;
				c2 = CGA_BLUE;
			} else if (phase == 1) {
				c1 = CGA_PINK;
				c2 = CGA_PINK;
			} else if (phase == 2) {
				c1 = CGA_BLUE;
				c2 = CGA_PINK;
			} else if (phase == 3) {
				c1 = CGA_BLUE;
				c2 = CGA_BLUE;
			}

			al_draw_filled_rectangle(x, y, x2, y2, c1); 
			al_draw_filled_rectangle(x, y2, x2, y3, c2);
			//al_draw_filled_rectangle(x, y, x2, y3, sine > 0.5 ? WHITE : CGA_BLUE);
			//al_draw_filled_rectangle(x, y, x2, y3, CGA_BLUE);
		}
		line_num++;
	}

	this->time += 0.1;

}

} // namespace dp::client::ui
