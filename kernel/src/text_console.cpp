//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <algorithms.hpp>

#include "text_console.hpp"

namespace {

enum vga_color {
    BLACK         = 0,
    BLUE          = 1,
    GREEN         = 2,
    CYAN          = 3,
    RED           = 4,
    MAGENTA       = 5,
    BROWN         = 6,
    LIGHT_GREY    = 7,
    DARK_GREY     = 8,
    LIGHT_BLUE    = 9,
    LIGHT_GREEN   = 10,
    LIGHT_CYAN    = 11,
    LIGHT_RED     = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_BROWN   = 14,
    WHITE         = 15,
};

uint8_t make_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

uint16_t make_vga_entry(char c, uint8_t color) {
    uint16_t c16     = c;
    uint16_t color16 = color;
    return c16 | color16 << 8;
}

} //end of anonymous namespace

void text_console::init() {
    //Nothing special to init
}

size_t text_console::lines() {
    return 25;
}

size_t text_console::columns() {
    return 80;
}

void text_console::clear() {
    for (int line = 0; line < 25; ++line) {
        for (uint64_t column = 0; column < 80; ++column) {
            print_char(line, column, ' ');
        }
    }
}

void text_console::scroll_up() {
    auto vga_buffer_fast = reinterpret_cast<uint64_t*>(0x0B8000);
    auto destination     = vga_buffer_fast;
    auto source          = &vga_buffer_fast[20];

    std::copy_n(source, 24 * 20, destination);

    auto vga_buffer = reinterpret_cast<uint16_t*>(0x0B8000);
    for (uint64_t i = 0; i < 80; ++i) {
        vga_buffer[24 * 80 + i] = make_vga_entry(' ', make_color(WHITE, BLACK));
    }
}

void text_console::print_char(size_t line, size_t column, char c) {
    uint16_t* vga_buffer = reinterpret_cast<uint16_t*>(0x0B8000);

    vga_buffer[line * 80 + column] = make_vga_entry(c, make_color(WHITE, BLACK));
}
