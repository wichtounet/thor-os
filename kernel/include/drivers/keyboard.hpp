//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <types.hpp>

#include <tlib/keycode.hpp>

namespace keyboard {

const char KEY_ENTER = 0x1C;
const char KEY_BACKSPACE = 0x0E;
const char KEY_UP = 0x48;
const char KEY_DOWN = 0x50;
const char KEY_LEFT_SHIFT = 0x2A;
const char KEY_RIGHT_SHIFT = 0x36;

const char KEY_LEFT_CTRL = 0x1D;

void install_driver();
char get_char();
char key_to_ascii(uint8_t key);
char shift_key_to_ascii(uint8_t key);

std::keycode raw_key_to_keycode(uint8_t key);

void get_char_blocking();

}

#endif
