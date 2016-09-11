//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "vesa_console.hpp"
#include "vesa.hpp"
#include "early_memory.hpp"

namespace {

constexpr const size_t MARGIN = 10;
constexpr const size_t PADDING = 5;
constexpr const size_t LEFT = MARGIN + PADDING;
constexpr const size_t TOP = 40;

size_t _lines;
size_t _columns;
uint32_t _color;

} //end of anonymous namespace

void vesa_console::init(){
    auto& block = *reinterpret_cast<vesa::mode_info_block_t*>(early::vesa_mode_info_address);

    _columns = (block.width - (MARGIN + PADDING) * 2) / 8;
    _lines = (block.height - TOP - MARGIN - PADDING) / 16;
    _color = vesa::make_color(255, 255, 255);

    vesa::draw_hline(MARGIN, MARGIN, block.width - 2 * MARGIN, _color);
    vesa::draw_hline(MARGIN, 35, block.width - 2 * MARGIN, _color);
    vesa::draw_hline(MARGIN, block.height - MARGIN, block.width - 2 * MARGIN, _color);

    vesa::draw_vline(MARGIN, MARGIN, block.height - 2 * MARGIN, _color);
    vesa::draw_vline(block.width - MARGIN, MARGIN, block.height - 2 * MARGIN, _color);

    vesa::draw_rect(200, 200, 100, 100, vesa::make_color(182, 148, 179));

    auto title_left = (block.width - 4 * 8) / 2;
    vesa::draw_char(title_left, PADDING + MARGIN, 'R', _color);
    vesa::draw_char(title_left + 8, PADDING + MARGIN, 'e', _color);
    vesa::draw_char(title_left + 16, PADDING + MARGIN, 'O', _color);
    vesa::draw_char(title_left + 24, PADDING + MARGIN, 'S', _color);
}

size_t vesa_console::lines(){
    return _lines;
}

size_t vesa_console::columns(){
    return _columns;
}

void vesa_console::clear(){
    vesa::draw_rect(LEFT, TOP, _columns * 8, _lines* 16, 0, 0, 0);
}

void vesa_console::scroll_up(){
    vesa::move_lines_up(TOP + 16, LEFT, _columns * 8, (_lines - 1) * 16, 16);
    vesa::draw_rect(LEFT, TOP + (_lines - 1) * 16, _columns * 8, 16, 0, 0, 0);
}

void vesa_console::print_char(size_t line, size_t column, char c){
    vesa::draw_char(LEFT + 8 * column, TOP + 16 * line, c, _color);
}
