//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "drivers/keyboard.hpp"

#include "interrupts.hpp"
#include "kernel_utils.hpp"
#include "stdio.hpp"
#include "logging.hpp"

using keycode = std::keycode;

namespace {

keycode pressed_codes[128] = {
    keycode::INVALID
    , keycode::PRESSED_ESC
    , keycode::PRESSED_1
    , keycode::PRESSED_2
    , keycode::PRESSED_3
    , keycode::PRESSED_4
    , keycode::PRESSED_5
    , keycode::PRESSED_6
    , keycode::PRESSED_7
    , keycode::PRESSED_8
    , keycode::PRESSED_9
    , keycode::PRESSED_0
    , keycode::PRESSED_DASH
    , keycode::PRESSED_EQUALS
    , keycode::PRESSED_BACKSPACE
    , keycode::PRESSED_TAB
    , keycode::PRESSED_Q
    , keycode::PRESSED_W
    , keycode::PRESSED_E
    , keycode::PRESSED_R
    , keycode::PRESSED_T
    , keycode::PRESSED_Z
    , keycode::PRESSED_U
    , keycode::PRESSED_I
    , keycode::PRESSED_O
    , keycode::PRESSED_P
    , keycode::PRESSED_LEFT_SQUARE_BRACKET
    , keycode::PRESSED_RIGHT_SQUARE_BRACKET
    , keycode::PRESSED_ENTER
    , keycode::PRESSED_CTRL
    , keycode::PRESSED_A
    , keycode::PRESSED_S
    , keycode::PRESSED_D
    , keycode::PRESSED_F
    , keycode::PRESSED_G
    , keycode::PRESSED_H
    , keycode::PRESSED_J
    , keycode::PRESSED_K
    , keycode::PRESSED_L
    , keycode::PRESSED_SEMICOLON
    , keycode::PRESSED_QUOTE
    , keycode::PRESSED_TICK
    , keycode::PRESSED_LEFT_SHIFT
    , keycode::PRESSED_BACKSLASH
    , keycode::PRESSED_Y
    , keycode::PRESSED_X
    , keycode::PRESSED_C
    , keycode::PRESSED_V
    , keycode::PRESSED_B
    , keycode::PRESSED_N
    , keycode::PRESSED_M
    , keycode::PRESSED_COMMA
    , keycode::PRESSED_DOT
    , keycode::PRESSED_SLASH
    , keycode::PRESSED_RIGHT_SHIFT
    , keycode::PRESSED_STAR
    , keycode::PRESSED_ALT
    , keycode::PRESSED_SPACE
    , keycode::PRESSED_CAPS_LOCK
    , keycode::PRESSED_F1
    , keycode::PRESSED_F2
    , keycode::PRESSED_F3
    , keycode::PRESSED_F4
    , keycode::PRESSED_F5
    , keycode::PRESSED_F6
    , keycode::PRESSED_F7
    , keycode::PRESSED_F8
    , keycode::PRESSED_F9
    , keycode::PRESSED_F10
    , keycode::PRESSED_NUM_LOCK
    , keycode::PRESSED_SCROLL_LOCK
    , keycode::PRESSED_KEY_7
    , keycode::PRESSED_KEY_8
    , keycode::PRESSED_KEY_9
    , keycode::PRESSED_KEY_MINUS
    , keycode::PRESSED_KEY_4
    , keycode::PRESSED_KEY_5
    , keycode::PRESSED_KEY_6
    , keycode::PRESSED_KEY_PLUS
    , keycode::PRESSED_KEY_1
    , keycode::PRESSED_KEY_2
    , keycode::PRESSED_KEY_3
    , keycode::PRESSED_KEY_0
    , keycode::PRESSED_KEY_DOT
    , keycode::INVALID
    , keycode::INVALID
    , keycode::INVALID
    , keycode::PRESSED_F11
    , keycode::PRESSED_F12
    , keycode::INVALID
};

keycode released_codes[128] = {
    keycode::INVALID
    , keycode::RELEASED_ESC
    , keycode::RELEASED_1
    , keycode::RELEASED_2
    , keycode::RELEASED_3
    , keycode::RELEASED_4
    , keycode::RELEASED_5
    , keycode::RELEASED_6
    , keycode::RELEASED_7
    , keycode::RELEASED_8
    , keycode::RELEASED_9
    , keycode::RELEASED_0
    , keycode::RELEASED_DASH
    , keycode::RELEASED_EQUALS
    , keycode::RELEASED_BACKSPACE
    , keycode::RELEASED_TAB
    , keycode::RELEASED_Q
    , keycode::RELEASED_W
    , keycode::RELEASED_E
    , keycode::RELEASED_R
    , keycode::RELEASED_T
    , keycode::RELEASED_Z
    , keycode::RELEASED_U
    , keycode::RELEASED_I
    , keycode::RELEASED_O
    , keycode::RELEASED_P
    , keycode::RELEASED_LEFT_SQUARE_BRACKET
    , keycode::RELEASED_RIGHT_SQUARE_BRACKET
    , keycode::RELEASED_ENTER
    , keycode::RELEASED_CTRL
    , keycode::RELEASED_A
    , keycode::RELEASED_S
    , keycode::RELEASED_D
    , keycode::RELEASED_F
    , keycode::RELEASED_G
    , keycode::RELEASED_H
    , keycode::RELEASED_J
    , keycode::RELEASED_K
    , keycode::RELEASED_L
    , keycode::RELEASED_SEMICOLON
    , keycode::RELEASED_QUOTE
    , keycode::RELEASED_TICK
    , keycode::RELEASED_LEFT_SHIFT
    , keycode::RELEASED_BACKSLASH
    , keycode::RELEASED_Y
    , keycode::RELEASED_X
    , keycode::RELEASED_C
    , keycode::RELEASED_V
    , keycode::RELEASED_B
    , keycode::RELEASED_N
    , keycode::RELEASED_M
    , keycode::RELEASED_COMMA
    , keycode::RELEASED_DOT
    , keycode::RELEASED_SLASH
    , keycode::RELEASED_RIGHT_SHIFT
    , keycode::RELEASED_STAR
    , keycode::RELEASED_ALT
    , keycode::RELEASED_SPACE
    , keycode::RELEASED_CAPS_LOCK
    , keycode::RELEASED_F1
    , keycode::RELEASED_F2
    , keycode::RELEASED_F3
    , keycode::RELEASED_F4
    , keycode::RELEASED_F5
    , keycode::RELEASED_F6
    , keycode::RELEASED_F7
    , keycode::RELEASED_F8
    , keycode::RELEASED_F9
    , keycode::RELEASED_F10
    , keycode::RELEASED_NUM_LOCK
    , keycode::RELEASED_SCROLL_LOCK
    , keycode::RELEASED_KEY_7
    , keycode::RELEASED_KEY_8
    , keycode::RELEASED_KEY_9
    , keycode::RELEASED_KEY_MINUS
    , keycode::RELEASED_KEY_4
    , keycode::RELEASED_KEY_5
    , keycode::RELEASED_KEY_6
    , keycode::RELEASED_KEY_PLUS
    , keycode::RELEASED_KEY_1
    , keycode::RELEASED_KEY_2
    , keycode::RELEASED_KEY_3
    , keycode::RELEASED_KEY_0
    , keycode::RELEASED_KEY_DOT
    , keycode::INVALID
    , keycode::INVALID
    , keycode::INVALID
    , keycode::RELEASED_F11
    , keycode::RELEASED_F12
    , keycode::INVALID
};

char qwertz[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '\'', '=', '\b', /* Backspace */
  '\t',         /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'z', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,          /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,        /* Left shift */
 '\\', 'y', 'x', 'c', 'v', 'b', 'n',            /* 49 */
  'm', ',', '.', '-',   0,              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

char shifted_qwertz[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '/', '8', /* 9 */
  '9', '0', '_', '=', '\b', /* Backspace */
  '\t',         /* Tab */
  'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Z', 'U', 'I', 'O', 'P', '[', ']', '\n', /* Enter key */
    0,          /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', /* 39 */
 '\'', '`',   0,        /* Left shift */
 '\\', 'Y', 'X', 'C', 'V', 'B', 'N',            /* 49 */
  'M', ',', ':', '/',   0,              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

void keyboard_handler(interrupt::syscall_regs*, void*){
    auto key = static_cast<char>(in_byte(0x60));

    stdio::get_active_terminal().send_input(key);
}

} //end of anonymous namespace

void keyboard::install_driver(){
    if(!interrupt::register_irq_handler(1, keyboard_handler, nullptr)){
        logging::logf(logging::log_level::ERROR, "kbd: Unable to register IRQ handler 1\n");
        return;
    }

    // At this point, we need to clear the keyboard buffer
    // Otherwise, all the following events will be lost
    unsigned char key = 0;
    while(((key = in_byte(0x64)) & 1) == 1){
        in_byte(0x60);
    }
}

char keyboard::key_to_ascii(uint8_t key){
    return qwertz[key];
}

char keyboard::shift_key_to_ascii(uint8_t key){
    return shifted_qwertz[key];
}

keycode keyboard::raw_key_to_keycode(uint8_t key){
    if(key & 0x80){
        key &= ~(0x80);
        return released_codes[key];
    } else {
        return pressed_codes[key];
    }
}
