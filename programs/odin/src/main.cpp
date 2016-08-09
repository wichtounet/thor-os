//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <system.hpp>
#include <graphics.hpp>
#include <print.hpp>
#include <malloc.hpp>

namespace {

uint32_t* z_buffer = nullptr;

uint64_t width = 0;
uint64_t height = 0;
uint64_t x_shift = 0;
uint64_t y_shift = 0;
uint64_t bytes_per_scan_line = 0;
uint64_t red_shift = 0;
uint64_t green_shift = 0;
uint64_t blue_shift = 0;

uint32_t make_color(uint8_t r, uint8_t g, uint8_t b){
    return (r << red_shift) + (g << green_shift) + (b << blue_shift);
}

void fill_buffer(uint32_t color){
    auto where = 0;
    for(size_t j = 0; j < height; ++j){
        for(size_t i = 0; i < width; ++i){
            z_buffer[where + i] = color;
        }

        where += y_shift;
    }
}

void draw_rect(size_t x, size_t y, size_t w, size_t h, uint32_t color){
    auto where = x + y * y_shift;

    for(size_t j = 0; j < h; ++j){
        for(size_t i = 0; i < w; ++i){
            z_buffer[where + i] = color;
        }

        where += y_shift;
    }
}

void paint_cursor(){
    auto x = graphics::mouse_x();
    auto y = graphics::mouse_y();

    auto color = make_color(255, 0, 0);

    draw_rect(x, y, 5, 5, color);
}

} // end of anonnymous namespace

int main(int /*argc*/, char* /*argv*/[]){
    width = graphics::get_width();
    height = graphics::get_height();
    x_shift = graphics::get_x_shift();
    y_shift = graphics::get_y_shift();
    bytes_per_scan_line = graphics::get_bytes_per_scan_line();
    red_shift = graphics::get_red_shift();
    green_shift = graphics::get_green_shift();
    blue_shift = graphics::get_blue_shift();

    size_t total_size = height * bytes_per_scan_line;

    printf("total_size: %u\n",total_size);

    auto buffer = new char[total_size];

    z_buffer = reinterpret_cast<uint32_t*>(buffer);

    auto white = make_color(255, 255, 255);

    while(true){
        fill_buffer(white);

        paint_cursor();

        graphics::redraw(buffer);
        sleep_ms(5);
    }

    delete[] buffer;

    exit(0);
}
