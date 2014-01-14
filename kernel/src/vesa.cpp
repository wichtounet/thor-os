//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "stl/types.hpp"

#include "vesa.hpp"
#include "paging.hpp"

#include "console.hpp"

/**
 * VESA Drawing implementation.
 *
 * This implemenation only supports 32bpp. Moreover, it also erases the
 * reserved bits of each pixel.
 */

namespace {

uint32_t* screen;

size_t x_shift;
size_t y_shift;

size_t red_shift;
size_t blue_shift;
size_t green_shift;

uint8_t font[256 * 8];

void init_font(){
    auto start = font + 8 * 'a';

    start[0] = 0x18;
    start[1] = 0x3C;
    start[2] = 0x66;
    start[3] = 0x7E;
    start[4] = 0x66;
    start[5] = 0x66;
    start[6] = 0x00;
    start[7] = 0x00;
}

#include "Liberation.c"

} //end of anonymous namespace

void vesa::init(){
    auto& block = vesa::mode_info_block;

    //Compute the address of the LFB
    screen = reinterpret_cast<uint32_t*>(block.linear_video_buffer);

    x_shift = 1;
    y_shift = block.bytes_per_scan_line / 4;

    red_shift = block.linear_red_mask_position;
    blue_shift = block.linear_blue_mask_position;
    green_shift = block.linear_green_mask_position;

    size_t total_size = static_cast<size_t>(block.height) * block.bytes_per_scan_line;

    auto first_page = reinterpret_cast<uintptr_t>(paging::page_align(screen));
    auto left_padding = reinterpret_cast<uintptr_t>(screen) - first_page;
    auto bytes = left_padding + paging::PAGE_SIZE + total_size;
    auto pages = (bytes / paging::PAGE_SIZE) + 1;

    paging::identity_map(reinterpret_cast<void*>(first_page), pages);

    init_font();
}

uint32_t vesa::make_color(uint8_t r, uint8_t g, uint8_t b){
    return (r << red_shift) + (g << green_shift) + (b << blue_shift);
}

void vesa::draw_pixel(size_t x, size_t y, uint32_t color){
    auto where = x + y * y_shift;
    screen[where] = color;
}

void vesa::draw_pixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b){
    draw_pixel(x, y, make_color(r, g, b));
}

void vesa::draw_hline(size_t x, size_t y, size_t w, uint32_t color){
    auto where = x + y * y_shift;

    for(size_t i = 0; i < w; ++i){
        screen[where + i] = color;
    }
}

void vesa::draw_hline(size_t x, size_t y, size_t w, uint8_t r, uint8_t g, uint8_t b){
    draw_hline(x, y, w, make_color(r, g, b));
}

void vesa::draw_vline(size_t x, size_t y, size_t h, uint32_t color){
    auto where = x + y * y_shift;

    for(size_t i = 0; i < h; ++i){
        screen[where + i * y_shift] = color;
    }
}

void vesa::draw_vline(size_t x, size_t y, size_t h, uint8_t r, uint8_t g, uint8_t b){
    draw_vline(x, y, h, make_color(r, g, b));
}

void vesa::draw_char(size_t x, size_t y, char c, uint32_t color){
    auto where = x + y * y_shift;

    auto font_char = &Liberation_VESA_data[c * 16];

    for(size_t i = 0; i < 16; ++i){
        for(size_t j = 0; j < 8; ++j){
            if(font_char[i] & (1 << (8 - j))){
                screen[where+j] = color;
            } else {
                screen[where+j] = 0;
            }
        }

        where += y_shift;
    }
}

void vesa::draw_char(size_t x, size_t y, char c, uint8_t r, uint8_t g, uint8_t b){
    draw_char(x, y, c, make_color(r, g, b));
}

void vesa::draw_rect(size_t x, size_t y, size_t w, size_t h, uint32_t color){
    auto where = x + y * y_shift;

    for(size_t j = 0; j < h; ++j){
        for(size_t i = 0; i < w; ++i){
            screen[where + i] = color;
        }

        where += y_shift;
    }
}

void vesa::draw_rect(size_t x, size_t y, size_t w, size_t h, uint8_t r, uint8_t g, uint8_t b){
    draw_rect(x, y, w, h, make_color(r, g, b));
}

void vesa::move_lines_up(size_t y, size_t x, size_t w, size_t lines, size_t n){
    for(size_t i = 0; i < lines; ++i){
        auto destination = reinterpret_cast<size_t*>(screen + (y - n + i) * y_shift + x);
        auto source = reinterpret_cast<size_t*>(screen + (y + i) * y_shift + x);

        for(size_t j = 0; j < w / 2; ++j){
            destination[j] = source[j];
        }
    }
}
