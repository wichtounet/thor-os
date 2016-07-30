//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "timer.hpp"
#include "scheduler.hpp"
#include "logging.hpp"
#include "kernel.hpp"   //suspend_boot

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
    if(!pit::install()){
        logging::logf(logging::log_level::ERROR, "Unable to install any timer driver\n");
        suspend_boot();
    }

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
        asm  volatile ("cli");

        if(_timer_countdown != 0){
            asm  volatile ("sti");
            asm  volatile ("nop");
            asm  volatile ("nop");
            asm  volatile ("nop");
            asm  volatile ("nop");
            asm  volatile ("nop");
        } else {
            break;
        }
    }

    asm  volatile ("sti");
}

uint64_t timer::ticks(){
    return _timer_ticks;
}

uint64_t timer::seconds(){
    return _timer_seconds;
}
