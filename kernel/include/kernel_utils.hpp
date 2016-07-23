//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include <types.hpp>

inline uint8_t in_byte(uint16_t _port){
    uint8_t rv;

    asm volatile ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

inline uint16_t in_word(uint16_t _port){
    uint16_t rv;

    asm volatile ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

inline uint32_t in_dword(uint16_t _port){
    uint32_t rv;

    asm volatile ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

inline uint64_t in_qword(uint16_t _port){
    uint64_t rv;

    asm volatile ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

inline void out_byte (uint16_t _port, uint8_t _data){
    asm volatile ("out %[port], %[data]"
        : /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

inline void out_word(uint16_t _port, uint16_t _data){
    asm volatile ("out %[port], %[data]"
        :  /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

inline void out_dword(uint16_t _port, uint32_t _data){
    asm volatile ("out %[port], %[data]"
        :  /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

inline void out_qword(uint16_t _port, uint64_t _data){
    asm volatile ("out %[port], %[data]"
        :  /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

inline uint16_t switch_endian_16(uint16_t nb) {
   return (nb>>8) | (nb<<8);
}

inline uint32_t switch_endian_32(uint32_t nb) {
   return ((nb>>24)&0xff)      |
          ((nb<<8)&0xff0000)   |
          ((nb>>8)&0xff00)     |
          ((nb<<24)&0xff000000);
}

void print_stack(const char* msg, size_t check);
#define SHOW_STACK(M) { size_t check = 0; asm volatile("mov %0, rsp;" : "=r" (check)); print_stack(((M)), check); }

#endif
