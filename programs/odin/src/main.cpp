//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <random.hpp>

#include <tlib/system.hpp>
#include <tlib/graphics.hpp>
#include <tlib/print.hpp>
#include <tlib/malloc.hpp>

// TODO The order of the windows should be maintained by an
// intrusive list

namespace {

uint32_t* z_buffer = nullptr;

uint64_t width               = 0;
uint64_t height              = 0;
uint64_t x_shift             = 0;
uint64_t y_shift             = 0;
uint64_t bytes_per_scan_line = 0;
uint64_t red_shift           = 0;
uint64_t green_shift         = 0;
uint64_t blue_shift          = 0;

#include <tlib/Liberation.inl>

uint32_t make_color(uint8_t r, uint8_t g, uint8_t b) {
    return (r << red_shift) + (g << green_shift) + (b << blue_shift);
}

void fill_buffer(uint32_t color) {
    auto where = 0;
    for (size_t j = 0; j < height; ++j) {
        for (size_t i = 0; i < width; ++i) {
            z_buffer[where + i] = color;
        }

        where += y_shift;
    }
}

void draw_pixel(size_t x, size_t y, uint32_t color) {
    if(x < width && y < height){
        z_buffer[x + y * y_shift] = color;
    }
}

void draw_hline(size_t x, size_t y, size_t w, uint32_t color) {
    auto where = x + y * y_shift;

    for (size_t i = 0; i < w && x + i < width; ++i) {
        z_buffer[where + i] = color;
    }
}

void draw_vline(size_t x, size_t y, size_t h, uint32_t color) {
    auto where = x + y * y_shift;

    for (size_t j = 0; j < h && y + j < height; ++j) {
        z_buffer[where] = color;
        where += y_shift;
    }
}

void draw_rect(size_t x, size_t y, size_t w, size_t h, uint32_t color) {
    auto where = x + y * y_shift;

    for (size_t j = 0; j < h && y + j < height; ++j) {
        for (size_t i = 0; i < w && x + i < width; ++i) {
            z_buffer[where + i] = color;
        }

        where += y_shift;
    }
}

void draw_char(size_t x, size_t y, char c, uint32_t color) {
    auto where = x + y * y_shift;

    auto font_char = &Liberation_VESA_data[c * 16];

    for (size_t i = 0; i < 16; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            if (font_char[i] & (1 << (8 - j))) {
                z_buffer[where + j] = color;
            }
        }

        where += y_shift;
    }
}

void draw_string(size_t x, size_t y, const char* s, uint32_t color) {
    while (*s) {
        draw_char(x, y, *s, color);
        ++s;
        x += 8;
    }
}

void paint_cursor() {
    auto x = tlib::graphics::mouse_x();
    auto y = tlib::graphics::mouse_y();

    auto color = make_color(20, 20, 20);

    draw_pixel(x, y, color);
    draw_hline(x, y + 1, 2, color);
    draw_hline(x, y + 2, 3, color);
    draw_hline(x, y + 3, 4, color);
    draw_hline(x, y + 4, 5, color);
    draw_hline(x, y + 5, 4, color);
    draw_pixel(x, y + 6, color);
    draw_hline(x + 2, y + 6, 2, color);
    draw_hline(x + 3, y + 7, 2, color);
    draw_hline(x + 3, y + 8, 2, color);
}

void paint_top_bar() {
    draw_rect(0, 0, width, 18, make_color(51, 51, 51));
    draw_rect(0, 18, width, 2, make_color(25, 25, 25));

    auto date = tlib::local_date();

    auto date_str = tlib::sprintf("%u.%u.%u %u:%u", size_t(date.day), size_t(date.month), size_t(date.year), size_t(date.hour), size_t(date.minutes));

    draw_char(2, 2, 'T', make_color(30, 30, 30));
    draw_string(width - 128, 2, date_str.c_str(), make_color(200, 200, 200));
}

struct window {
private:
    size_t x;
    size_t y;
    size_t width;
    size_t height;

    uint32_t color;
    uint32_t border_color;

    bool drag            = false;
    int64_t drag_start_x = 0;
    int64_t drag_start_y = 0;

public:
    // TODO Relax std::vector to allow for non-default-constructible
    window() {}

    window(size_t x, size_t y, size_t width, size_t height)
            : x(x), y(y), width(width), height(height) {
        std::default_random_engine eng(tlib::ms_time());
        std::uniform_int_distribution<> color_dist(0, 255);

        color        = make_color(color_dist(eng), color_dist(eng), color_dist(eng));
        border_color = make_color(color_dist(eng), color_dist(eng), color_dist(eng));
    }

    window(const window& rhs) = default;
    window(window&& rhs) = default;

    window& operator=(const window& rhs) = default;
    window& operator=(window&& rhs) = default;

    void update() {
        if (drag) {
            auto mouse_x = tlib::graphics::mouse_x();
            auto mouse_y = tlib::graphics::mouse_y();

            auto delta_x = int64_t(mouse_x) - drag_start_x;
            auto delta_y = int64_t(mouse_y) - drag_start_y;

            if (int64_t(x) + delta_x < 0) {
                x = 0;
            } else {
                x += delta_x;

                x = std::min(x, ::width - width);
            }

            if (int64_t(y) + delta_y < 20) {
                y = 20;
            } else {
                y += delta_y;

                y = std::min(y, ::height - height);
            }

            drag_start_x = mouse_x;
            drag_start_y = mouse_y;
        }
    }

    void draw() const {
        constexpr const size_t border = 2;

        // Draw the background of the window
        draw_rect(x + 2, y + 2, width - 4, height - 4, color);

        // Draw the outer border
        draw_rect(x, y, width, border, border_color);
        draw_rect(x, y, border, height, border_color);
        draw_rect(x, y + height - border, width, border, border_color);
        draw_rect(x + width - border, y, border, height, border_color);

        // Draw the base line of the title bar
        draw_rect(x, y + 18, width, border, border_color);
    }

    bool mouse_in_title() {
        auto mouse_x = tlib::graphics::mouse_x();
        auto mouse_y = tlib::graphics::mouse_y();

        return mouse_x >= x && mouse_x <= x + width && mouse_y >= y + 2 && mouse_y <= y + 18;
    }

    bool inside(size_t look_x, size_t look_y) {
        return look_x >= x && look_x <= x + width && look_y >= y && look_y <= y + height;
    }

    void start_drag() {
        if (!drag) {
            drag         = true;
            drag_start_x = tlib::graphics::mouse_x();
            drag_start_y = tlib::graphics::mouse_y();
        }
    }

    void stop_drag() {
        drag = false;
    }
};

std::vector<window> windows;

void raise() {
    auto mouse_x = tlib::graphics::mouse_x();
    auto mouse_y = tlib::graphics::mouse_y();

    for (auto it = windows.begin(); it != windows.end(); ++it) {
        auto& window = *it;

        if (window.inside(mouse_x, mouse_y)) {
            //If the window is not the first, we raise it
            if (it != windows.begin()) {
                auto copy = window;
                windows.erase(it);
                windows.push_front(copy);
            }

            break;
        }
    }
}

} // end of anonnymous namespace

int main(int /*argc*/, char* /*argv*/ []) {
    width               = tlib::graphics::get_width();
    height              = tlib::graphics::get_height();
    x_shift             = tlib::graphics::get_x_shift();
    y_shift             = tlib::graphics::get_y_shift();
    bytes_per_scan_line = tlib::graphics::get_bytes_per_scan_line();
    red_shift           = tlib::graphics::get_red_shift();
    green_shift         = tlib::graphics::get_green_shift();
    blue_shift          = tlib::graphics::get_blue_shift();

    size_t total_size = height * bytes_per_scan_line;

    auto buffer = new char[total_size];

    z_buffer = reinterpret_cast<uint32_t*>(buffer);

    auto background = make_color(128, 128, 128);

    tlib::set_canonical(false);
    tlib::set_mouse(true);

    static constexpr const size_t sleep_timeout = 50;

    std::default_random_engine eng(tlib::ms_time());
    std::uniform_int_distribution<> width_dist(200, 300);
    std::uniform_int_distribution<> height_dist(100, 250);
    std::uniform_int_distribution<> position_dist(0, 500);

    while (true) {
        fill_buffer(background);

        paint_top_bar();

        for (auto& window : windows) {
            window.update();
        }

        // Draw the window from back to front
        std::for_each(windows.rbegin(), windows.rend(), [](window& window) {
            window.draw();
        });

        paint_cursor();

        tlib::graphics::redraw(buffer);

        auto before = tlib::ms_time();
        auto code   = tlib::read_input_raw(sleep_timeout);
        auto after  = tlib::ms_time();

        if (code != std::keycode::TIMEOUT) {
            switch (code) {
                case std::keycode::RELEASED_ENTER: {
                    size_t width  = width_dist(eng);
                    size_t height = width_dist(eng);
                    size_t pos_x  = position_dist(eng);
                    size_t pos_y  = position_dist(eng);

                    windows.emplace_back(width, height, pos_x, pos_y);

                    break;
                }

                case std::keycode::MOUSE_LEFT_PRESS:
                    tlib::user_logf("odin: left press");

                    raise();

                    if (windows.front().mouse_in_title()) {
                        tlib::user_logf("odin: start drag");
                        windows.front().start_drag();
                    }

                    break;

                case std::keycode::MOUSE_LEFT_RELEASE:
                    tlib::user_logf("odin: left release");

                    raise();

                    windows.front().stop_drag();

                    break;

                case std::keycode::MOUSE_RIGHT_PRESS:
                    raise();

                    tlib::user_logf("odin: right press");
                    break;

                case std::keycode::MOUSE_RIGHT_RELEASE:
                    raise();

                    tlib::user_logf("odin: right release");
                    break;

                default:
                    tlib::user_logf("odin: %u ", static_cast<size_t>(code));
            }

            auto duration = after - before;

            if (duration < sleep_timeout) {
                tlib::sleep_ms(sleep_timeout - duration);
            }
        }
    }

    tlib::set_canonical(true);
    tlib::set_mouse(false);

    delete[] buffer;

    return 0;
}
