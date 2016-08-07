//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VESA_H
#define VESA_H

#include "vesa_types.hpp"

namespace vesa {

bool init();
bool enabled();
void disable();

uint32_t make_color(uint8_t r, uint8_t g, uint8_t b);

void draw_pixel(size_t x, size_t y, uint32_t color);
void draw_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b);

void draw_hline(size_t x, size_t y, size_t w, uint8_t r, uint8_t g, uint8_t b);
void draw_hline(size_t x, size_t y, size_t w, uint32_t color);

void draw_vline(size_t x, size_t y, size_t h, uint32_t color);
void draw_vline(size_t x, size_t y, size_t h, uint8_t r, uint8_t g, uint8_t b);

void draw_rect(size_t x, size_t y, size_t w, size_t h, uint32_t color);
void draw_rect(size_t x, size_t y, size_t w, size_t h, uint8_t r, uint8_t g, uint8_t b);

void draw_char(size_t x, size_t y, char c, uint32_t color);
void draw_char(size_t x, size_t y, char c, uint8_t r, uint8_t g, uint8_t b);

void move_lines_up(size_t y, size_t x, size_t w, size_t lines, size_t n);

uint64_t get_width();
uint64_t get_height();

uint64_t get_x_shift();
uint64_t get_y_shift();

uint64_t get_bytes_per_scan_line();

uint64_t get_red_shift();
uint64_t get_green_shift();
uint64_t get_blue_shift();

void redraw(const char* buffer);

} //end of vesa namespace

#endif
