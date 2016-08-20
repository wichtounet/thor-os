//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/system.hpp>
#include <tlib/graphics.hpp>
#include <tlib/print.hpp>
#include <tlib/malloc.hpp>

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

#include <tlib/Liberation.inl>

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

void draw_pixel(size_t x, size_t y, uint32_t color){
    z_buffer[x + y * y_shift] = color;
}

void draw_hline(size_t x, size_t y, size_t w, uint32_t color){
    auto where = x + y * y_shift;

    for(size_t i = 0; i < w; ++i){
        z_buffer[where + i] = color;
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

void draw_char(size_t x, size_t y, char c, uint32_t color){
    auto where = x + y * y_shift;

    auto font_char = &Liberation_VESA_data[c * 16];

    for(size_t i = 0; i < 16; ++i){
        for(size_t j = 0; j < 8; ++j){
            if(font_char[i] & (1 << (8 - j))){
                z_buffer[where+j] = color;
            }
        }

        where += y_shift;
    }
}

void draw_string(size_t x, size_t y, const char* s, uint32_t color){
    while(*s){
        draw_char(x, y, *s, color);
        ++s;
        x += 8;
    }
}

void paint_cursor(){
    auto x = graphics::mouse_x();
    auto y = graphics::mouse_y();

    auto color = make_color(20, 20, 20);

    draw_pixel(x, y, color);
    draw_hline(x, y+1, 2, color);
    draw_hline(x, y+2, 3, color);
    draw_hline(x, y+3, 4, color);
    draw_hline(x, y+4, 5, color);
    draw_hline(x, y+5, 4, color);
    draw_pixel(x, y+6, color);
    draw_hline(x+2, y+6, 2, color);
    draw_hline(x+3, y+7, 2, color);
    draw_hline(x+3, y+8, 2, color);
}

void paint_top_bar(){
    draw_rect(0, 0, width, 18, make_color(51, 51, 51));
    draw_rect(0, 18, width, 2, make_color(25, 25, 25));

    auto date = local_date();

    auto date_str = sprintf("%u.%u.%u %u:%u", size_t(date.day), size_t(date.month), size_t(date.year), size_t(date.hour), size_t(date.minutes));

    draw_char(2, 2, 'T', make_color(30, 30, 30));
    draw_string(width - 128, 2, date_str.c_str(), make_color(200, 200, 200));
}

struct window {
    size_t x;
    size_t y;
    size_t width;
    size_t height;
    uint32_t color;

    // TODO Relax std::vector to allow for non-default-constructible
    window(){}

    window(size_t x, size_t y, size_t width, size_t height) : x(x), y(y), width(width), height(height) {
        color = make_color(25, 25, 155);
    }

    void draw(){
        draw_rect(x, y, width, height, color);
    }
};

std::vector<window> windows;

} // end of anonnymous namespace

int main(int /*argc*/, char* /*argv*/[]){
    // Create a default window
    windows.emplace_back(250, 250, 200, 400);

    width = graphics::get_width();
    height = graphics::get_height();
    x_shift = graphics::get_x_shift();
    y_shift = graphics::get_y_shift();
    bytes_per_scan_line = graphics::get_bytes_per_scan_line();
    red_shift = graphics::get_red_shift();
    green_shift = graphics::get_green_shift();
    blue_shift = graphics::get_blue_shift();

    size_t total_size = height * bytes_per_scan_line;

    auto buffer = new char[total_size];

    z_buffer = reinterpret_cast<uint32_t*>(buffer);

    auto background = make_color(211, 211, 211);

    set_canonical(false);
    set_mouse(true);

    static constexpr const size_t sleep_timeout = 50;

    while(true){
        fill_buffer(background);

        paint_top_bar();

        for(auto& window : windows){
            window.draw();
        }

        paint_cursor();

        graphics::redraw(buffer);

        auto before = ms_time();
        auto code = read_input_raw(sleep_timeout);
        auto after = ms_time();

        if(code != keycode::TIMEOUT){
            // TODO Handle event at this point

            switch(code){
                case keycode::MOUSE_LEFT_PRESS:
                    user_logf("odin: left press");
                    break;

                case keycode::MOUSE_LEFT_RELEASE:
                    user_logf("odin: left release");
                    break;

                case keycode::MOUSE_RIGHT_PRESS:
                    user_logf("odin: right press");
                    break;

                case keycode::MOUSE_RIGHT_RELEASE:
                    user_logf("odin: right release");
                    break;

                default:
                    user_logf("odin: %u ", static_cast<size_t>(code));
            }

            auto duration = after - before;

            if(duration < sleep_timeout){
                sleep_ms(sleep_timeout - duration);
            }
        }
    }

    set_canonical(true);
    set_mouse(false);

    delete[] buffer;

    return 0;
}
