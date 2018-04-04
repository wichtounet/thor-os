//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VESA_H
#define VESA_H

#include "vesa_types.hpp"

namespace vesa {

bool init();
bool enabled();
void disable();

uint32_t make_color(uint8_t r, uint8_t g, uint8_t b);

void draw_hline(size_t x, size_t y, size_t w, uint32_t color);
void draw_hline(void* buffer, size_t x, size_t y, size_t w, uint32_t color);

void draw_vline(size_t x, size_t y, size_t h, uint32_t color);
void draw_vline(void* buffer, size_t x, size_t y, size_t h, uint32_t color);

void draw_rect(size_t x, size_t y, size_t w, size_t h, uint32_t color);
void draw_rect(void* buffer, size_t x, size_t y, size_t w, size_t h, uint32_t color);

void draw_char(size_t x, size_t y, char c, uint32_t color);
void draw_char(void* buffer, size_t x, size_t y, char c, uint32_t color);

void move_lines_up(size_t y, size_t x, size_t w, size_t lines, size_t n);
void move_lines_up(void* buffer, size_t y, size_t x, size_t w, size_t lines, size_t n);

uint64_t get_width();
uint64_t get_height();

uint64_t get_x_shift();
uint64_t get_y_shift();

uint64_t get_bytes_per_scan_line();

uint64_t get_red_shift();
uint64_t get_green_shift();
uint64_t get_blue_shift();

void* create_buffer();

void redraw(const char* buffer);
void save(char* buffer);

} //end of vesa namespace

#endif
