#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include "types.hpp"

template<uint8_t IRQ>
void register_irq_handler(void (*handler)()){
    asm ("mov r8, %0; mov r9, %1; int 61"
        :
        : "I" (IRQ), "r" (handler)
        : "r8", "r9"
        );
}

template<uint8_t INT>
void interrupt(){
    asm ("int %0"
        :
        : "i" (INT)
        );
}

uint8_t in_byte(uint16_t _port);
void out_byte (uint16_t _port, uint8_t _data);

#endif
