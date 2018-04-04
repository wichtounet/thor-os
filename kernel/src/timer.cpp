//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "timer.hpp"
#include "scheduler.hpp"
#include "logging.hpp"
#include "kernel.hpp"   //suspend_boot

#include "drivers/pit.hpp"
#include "drivers/hpet.hpp"

#include "fs/sysfs.hpp"

namespace {

volatile uint64_t _timer_ticks = 0;
volatile uint64_t _timer_seconds = 0;
volatile uint64_t _timer_milliseconds = 0;
uint64_t _timer_frequency = 0;

uint64_t (*_counter_fun)() = nullptr;
uint64_t _counter_frequency = 0;

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

    sysfs::set_dynamic_value(sysfs::get_sys_path(), path("/uptime"), &sysfs_uptime);
}

void timer::tick(){
    // Simply let the scheduler know about the tick
    scheduler::tick();
}

uint64_t timer::seconds(){
    return counter() / counter_frequency();
}

uint64_t timer::milliseconds(){
    return counter() / (counter_frequency() / 1000);
}

uint64_t timer::timer_frequency(){
    return _timer_frequency;
}

void timer::timer_frequency(uint64_t freq){
    auto old_frequency = _timer_frequency;

    _timer_frequency = freq;

    logging::logf(logging::log_level::DEBUG, "timer: Frequency set to %u Hz\n", freq);

    scheduler::frequency_updated(old_frequency, _timer_frequency);
}

uint64_t timer::counter(){
    return _counter_fun();
}

uint64_t timer::counter_frequency(){
    return _counter_frequency;
}

void timer::counter_frequency(uint64_t freq){
    _counter_frequency = freq;
}

void timer::counter_fun(uint64_t (*fun)()){
    _counter_fun = fun;
}
