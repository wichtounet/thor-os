//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "timer.hpp"
#include "scheduler.hpp"
#include "kernel_utils.hpp"

#include "drivers/pit.hpp"

#include "fs/sysfs.hpp"

namespace {

uint64_t _timer_ticks = 0;
uint64_t _timer_seconds = 0;

volatile uint64_t _timer_countdown = 0;

std::string sysfs_uptime(){
    return std::to_string(timer::seconds());
}

} //End of anonymous namespace

void timer::install(){
    pit::install();

    sysfs::set_dynamic_value("/sys/", "uptime", &sysfs_uptime);
}

void timer::tick(){
    ++_timer_ticks;

    if(_timer_countdown != 0){
        --_timer_countdown;
    }

    scheduler::tick();

    if(_timer_ticks % 1000 == 0){
        ++_timer_seconds;
    }
}

void timer::sleep_ms(uint64_t delay){
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

uint64_t timer::ticks(){
    return _timer_ticks;
}

uint64_t timer::seconds(){
    return _timer_seconds;
}
