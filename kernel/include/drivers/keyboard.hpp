//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <types.hpp>

#include <tlib/keycode.hpp>

namespace keyboard {

const char KEY_ENTER       = 0x1C;
const char KEY_BACKSPACE   = 0x0E;
const char KEY_UP          = 0x48;
const char KEY_DOWN        = 0x50;
const char KEY_LEFT_SHIFT  = 0x2A;
const char KEY_RIGHT_SHIFT = 0x36;
const char KEY_LEFT_CTRL   = 0x1D;
const char KEY_ALT         = 56;
const char KEY_F1          = 59;
const char KEY_F2          = 60;
const char KEY_F3          = 61;

/*!
 * \brief Install the keyboard driver
 */
void install_driver();

char key_to_ascii(uint8_t key);
char shift_key_to_ascii(uint8_t key);

std::keycode raw_key_to_keycode(uint8_t key);

}

#endif
