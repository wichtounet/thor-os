//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <types.hpp>

#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

namespace graphics {

uint64_t get_width();
uint64_t get_height();

uint64_t get_x_shift();
uint64_t get_y_shift();

uint64_t get_bytes_per_scan_line();

uint64_t get_red_shift();
uint64_t get_green_shift();
uint64_t get_blue_shift();

uint64_t mouse_x();
uint64_t mouse_y();

void redraw(char* buffer);

} // end of namespace graphics

} // end of namespace tlib

#endif
