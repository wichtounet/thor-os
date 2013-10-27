#include "timer.hpp"

#include "kernel_utils.hpp"

namespace {

std::size_t _timer_ticks;
std::size_t _timer_seconds;

void timer_handler(){
    ++_timer_ticks;

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

std::size_t timer_ticks(){
    return _timer_ticks;
}

std::size_t timer_seconds(){
    return _timer_seconds;
}
