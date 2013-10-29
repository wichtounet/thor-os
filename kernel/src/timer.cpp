#include "timer.hpp"

#include "kernel_utils.hpp"

namespace {

std::size_t _timer_ticks = 0;
std::size_t _timer_seconds = 0;

volatile std::size_t _timer_countdown = 0;

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
    std::size_t divisor = 1193180 / 1000;

    out_byte(0x43, 0x36);
    out_byte(0x40, (uint8_t) divisor);
    out_byte(0x40, (uint8_t) (divisor >> 8));

    register_irq_handler<0>(timer_handler);
}

void sleep_ms(std::size_t delay){
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

std::size_t timer_ticks(){
    return _timer_ticks;
}

std::size_t timer_seconds(){
    return _timer_seconds;
}
