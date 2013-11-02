#include "kernel_utils.hpp"

uint8_t in_byte(uint16_t _port){
    uint8_t rv;
    __asm__ __volatile__ ("in %0, %1" : "=a" (rv) : "dN" (_port));
    return rv;
}

void out_byte (uint16_t _port, uint8_t _data){
    __asm__ __volatile__ ("out %0, %1" : : "dN" (_port), "a" (_data));
}

uint16_t in_word(uint16_t _port){
    uint16_t rv;
    __asm__ __volatile__ ("in %0, %1" : "=a" (rv) : "dN" (_port));
    return rv;
}

void out_word(uint16_t _port, uint16_t _data){
    __asm__ __volatile__ ("out %0, %1" : : "dN" (_port), "a" (_data));
}
