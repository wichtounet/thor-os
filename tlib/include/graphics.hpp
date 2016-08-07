//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <types.hpp>

namespace graphics {

uint64_t get_width();
uint64_t get_height();

uint64_t get_x_shift();
uint64_t get_y_shift();

uint64_t get_bytes_per_scan_line();

uint64_t get_red_shift();
uint64_t get_green_shift();
uint64_t get_blue_shift();

void redraw(char* buffer);

} // end of namespace graphics

#endif
