#include "timer.hpp"

#include "kernel_utils.hpp"

namespace {

uint64_t _timer_ticks = 0;
uint64_t _timer_seconds = 0;

volatile uint64_t _timer_countdown = 0;

void timer_handler(){
    ++_timer_ticks;

    if(_timer_countdown != 0){
        --_timer_countdown;
    }

    if(_timer_ticks % 1000 == 0){
        ++_timer_seconds;
    }
}

}

void install_timer(){
    uint64_t divisor = 1193180 / 1000;

    out_byte(0x43, 0x36);
    out_byte(0x40, static_cast<uint8_t>(divisor));
    out_byte(0x40, static_cast<uint8_t>(divisor >> 8));

    register_irq_handler<0>(timer_handler);
}

void sleep_ms(uint64_t delay){
    _timer_countdown = delay;

    while(true){
        __asm__  __volatile__ ("cli");

        if(_timer_countdown != 0){
            __asm__  __volatile__ ("sti");
            __asm__  __volatile__ ("nop");
            __asm__  __volatile__ ("nop");
            __asm__  __volatile__ ("nop");
            __asm__  __volatile__ ("nop");
            __asm__  __volatile__ ("nop");
        } else {
            break;
        }
    }

    __asm__  __volatile__ ("sti");
}

uint64_t timer_ticks(){
    return _timer_ticks;
}

uint64_t timer_seconds(){
    return _timer_seconds;
}
