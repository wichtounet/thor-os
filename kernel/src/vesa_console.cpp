//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "vesa_console.hpp"
#include "vesa.hpp"

namespace {

constexpr const size_t MARGIN = 10;
constexpr const size_t PADDING = 5;
constexpr const size_t LEFT = MARGIN + PADDING;
constexpr const size_t TOP = 50;

size_t _lines;
size_t _columns;
uint32_t _color;

} //end of anonymous namespace

void vesa_console::init(){
    auto& block = vesa::mode_info_block;

    _columns = (block.width - (MARGIN + PADDING) * 2) / 8;
    _lines = (block.height - TOP - MARGIN - PADDING) / 16;
    _color = vesa::make_color(0, 255, 0);

    vesa::draw_hline(10, 10, 1004, _color);
    vesa::draw_hline(10, 40, 1004, _color);
    vesa::draw_hline(10, 758, 1004, _color);

    vesa::draw_vline(10, 10, 748, _color);
    vesa::draw_vline(1014, 10, 748, _color);
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

}

void vesa_console::print_char(size_t line, size_t column, char c){
    vesa::draw_char(LEFT + 8 * column, TOP + 16 * line, c, _color);
}
