//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.hpp"

namespace keyboard {

const char KEY_ENTER = 0x1C;
const char KEY_BACKSPACE = 0x0E;
const char KEY_UP = 0x48;

void install_driver();
char get_char();
char key_to_ascii(uint8_t key);

}

#endif
