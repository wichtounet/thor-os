//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "serial.hpp"
#include "kernel_utils.hpp"

#define COM1_PORT 0x3f8

void serial::init() {
   out_byte(COM1_PORT + 1, 0x00);    // Disable all interrupts
   out_byte(COM1_PORT + 3, 0x80);    // Enable DLAB
   out_byte(COM1_PORT + 0, 0x03);    // 38400 baud
   out_byte(COM1_PORT + 1, 0x00);
   out_byte(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   out_byte(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   out_byte(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

bool serial::is_transmit_buffer_empty() {
   return in_byte(COM1_PORT + 5) & 0x20;
}

void serial::transmit(char a) {
   while (is_transmit_buffer_empty() == 0){}

   out_byte(COM1_PORT,a);
}
