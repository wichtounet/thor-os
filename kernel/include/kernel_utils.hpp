//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#ifndef THOR_INIT
#include <types.hpp>
#endif

inline uint8_t in_byte(uint16_t _port){
    uint8_t rv;

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

inline uint16_t in_word(uint16_t _port){
    uint16_t rv;

    asm volatile ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

inline void out_word(uint16_t _port, uint16_t _data){
    asm volatile ("out %[port], %[data]"
        :  /* No outputs */
        : [port] "dN" (_port), [data] "a" (_data));
}

#ifndef THOR_INIT

inline uint32_t in_dword(uint16_t _port){
    uint32_t rv;

    asm volatile ("in %[data], %[port]"
        : [data] "=a" (rv)
        : [port] "dN" (_port));

    return rv;
}

inline void out_dword(uint16_t _port, uint32_t _data){
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

#endif //THOR_INIT

#endif
