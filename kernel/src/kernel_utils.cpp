#include "kernel_utils.hpp"

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
