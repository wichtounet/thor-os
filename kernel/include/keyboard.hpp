#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.hpp"

namespace keyboard {

const char KEY_ENTER = 0x1C;
const char KEY_BACKSPACE = 0x0E;

void install_driver();
char get_char();
char key_to_ascii(uint8_t key);

}

#endif
