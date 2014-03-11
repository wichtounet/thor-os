//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "kernel_utils.hpp"
#include "console.hpp"

uint8_t in_byte(uint16_t _port){
    uint8_t rv;

    __asm__ __volatile__ ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

uint16_t in_word(uint16_t _port){
    uint16_t rv;

    __asm__ __volatile__ ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

void out_byte (uint16_t _port, uint8_t _data){
    __asm__ __volatile__ ("out %[port], %[data]"
        : /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

void out_word(uint16_t _port, uint16_t _data){
    __asm__ __volatile__ ("out %[port], %[data]"
        :  /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

void print_stack(const char* s, size_t check){
    printf("%s stack: %u (16B-a:%u) \n", s, check, static_cast<size_t>(check % 16));
}
