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
#include "drivers/hpet.hpp"

#include "fs/sysfs.hpp"

namespace {

uint64_t _timer_ticks = 0;
uint64_t _timer_seconds = 0;
uint64_t _timer_frequency = 0;

//TODO The uptime in seconds with HPET is not correct
std::string sysfs_uptime(){
    return std::to_string(timer::seconds());
}

bool find_hw_timer(){
    if(hpet::install()){
        return true;
    } else {
        logging::logf(logging::log_level::DEBUG, "timer: Unable to install HPET driver\n");
    }

    if(pit::install()){
        return true;
    } else {
        logging::logf(logging::log_level::DEBUG, "timer: Unable to install PIT driver\n");
    }

    return false;
}

} //End of anonymous namespace

void timer::install(){
    if(!find_hw_timer()){
        logging::logf(logging::log_level::ERROR, "Unable to install any timer driver\n");
        suspend_boot();
    }

    sysfs::set_dynamic_value("/sys/", "uptime", &sysfs_uptime);
}

void timer::tick(){
    ++_timer_ticks;

    scheduler::tick();

    if(_timer_ticks % frequency() == 0){
        ++_timer_seconds;
    }
}

uint64_t timer::ticks(){
    return _timer_ticks;
}

uint64_t timer::seconds(){
    return _timer_seconds;
}

uint64_t timer::frequency(){
    return _timer_frequency;
}

void timer::frequency(uint64_t freq){
    auto old_frequency = _timer_frequency;

    _timer_frequency = freq;

    logging::logf(logging::log_level::DEBUG, "timer: Frequency set to %u Hz\n", freq);

    scheduler::frequency_updated(old_frequency, _timer_frequency);
}
