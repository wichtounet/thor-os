//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "drivers/pit.hpp"

#include "timer.hpp"
#include "interrupts.hpp"
#include "scheduler.hpp"
#include "kernel_utils.hpp"
#include "logging.hpp"

namespace {

size_t pit_counter = 0;

void timer_handler(interrupt::syscall_regs*, void*){
    ++pit_counter;

    timer::tick();
}

} //End of anonymous namespace

bool pit::install(){
    uint64_t divisor = 1193180 / 1000;

    out_byte(0x43, 0x36);
    out_byte(0x40, static_cast<uint8_t>(divisor));
    out_byte(0x40, static_cast<uint8_t>(divisor >> 8));

    // Indicate the timer frequency
    timer::timer_frequency(1000);

    if(!interrupt::register_irq_handler(0, timer_handler, nullptr)){
        logging::logf(logging::log_level::ERROR, "Unable to register PIT IRQ handler 0\n");

        return false;
    }

    // Let the timer know how to query the counter
    timer::counter_fun(pit::counter);

    logging::logf(logging::log_level::TRACE, "PIT Driver Installed\n");

    return true;
}

void pit::remove(){
    if(!interrupt::unregister_irq_handler(0, timer_handler)){
        logging::logf(logging::log_level::ERROR, "Unable to unregister PIT IRQ handler 0\n");
    }

    logging::logf(logging::log_level::TRACE, "PIT Driver Removed\n");
}

uint64_t pit::counter(){
    return pit_counter;
}
