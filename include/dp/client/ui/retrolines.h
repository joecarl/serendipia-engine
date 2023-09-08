#ifndef RETROLINES_H
#define RETROLINES_H

#include <cstdint>
#include <vector>
#include <string>


namespace dp::client::ui {

typedef struct {
	uint8_t x;
	uint8_t width;
} Dash;

typedef std::vector<Dash> RetroLine;


class RetroLines {

	std::vector<RetroLine> lines;

	uint8_t width = 0;

	uint8_t mult_x = 6;

	uint8_t mult_y = 4;

	float time = 0;

	void calc_width();

public:

	RetroLines(std::vector<RetroLine>&& _lines);

	RetroLines(std::vector<std::string>&& str_lines);

	void draw(float ox, float oy);

	uint8_t get_width() { return this->width; }

};

} // namespace dp::client::ui

#endif