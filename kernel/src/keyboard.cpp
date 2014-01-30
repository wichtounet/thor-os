//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "keyboard.hpp"
#include "interrupts.hpp"
#include "kernel_utils.hpp"
#include "semaphore.hpp"
#include "spinlock.hpp"
#include "sleep_queue.hpp"

namespace {

char qwertz[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'z', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'y', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

char shifted_qwertz[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Z', 'U', 'I', 'O', 'P', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'Y', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

const uint8_t BUFFER_SIZE = 64;

//TODO Encapsulate the buffer
char input_buffer[BUFFER_SIZE];
volatile uint8_t start;
volatile uint8_t count;

spinlock lock;
sleep_queue queue;

void give_char(scheduler::pid_t pid, char t){
    scheduler::get_process(pid).regs.rax = t;
}

void keyboard_handler(const interrupt::syscall_regs&){
    auto key = static_cast<char>(in_byte(0x60));

    std::lock_guard<spinlock> l(lock);

    if(count == BUFFER_SIZE){
        //The buffer is full, we loose the characters
    } else {
        if(queue.empty()){
            auto end = (start + count) % BUFFER_SIZE;
            input_buffer[end] = key;
            ++count;
        } else {
            auto pid = queue.wake_up();
            give_char(pid, keyboard::shift_key_to_ascii(key));
        }
    }
}

} //end of anonymous namespace

void keyboard::install_driver(){
    interrupt::register_irq_handler(1, keyboard_handler);

    start = 0;
    count = 0;
}

void keyboard::get_char_blocking(){
    std::lock_guard<spinlock> l(lock);

    if(count > 0){
        auto key = input_buffer[start];
        start = (start + 1) % BUFFER_SIZE;
        --count;

        auto pid = scheduler::get_pid();
        give_char(pid, shift_key_to_ascii(key));
    } else {
        //Wait for a char
        queue.sleep();
    }
}

//TODO Once shell is user mode, can be removed
char keyboard::get_char(){
    //Wait for the buffer to contains something
    while(count == 0){
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
        __asm__  __volatile__ ("nop");
    }

    auto key = input_buffer[start];
    start = (start + 1) % BUFFER_SIZE;
    --count;

    return key;
}

char keyboard::key_to_ascii(uint8_t key){
    return qwertz[key];
}

char keyboard::shift_key_to_ascii(uint8_t key){
    return shifted_qwertz[key];
}
