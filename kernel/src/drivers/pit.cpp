//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "drivers/pit.hpp"

#include "timer.hpp"
#include "interrupts.hpp"
#include "scheduler.hpp"
#include "kernel_utils.hpp"

namespace {

void timer_handler(interrupt::syscall_regs*){
    timer::tick();
}

} //End of anonymous namespace

void pit::install(){
    uint64_t divisor = 1193180 / 1000;

    out_byte(0x43, 0x36);
    out_byte(0x40, static_cast<uint8_t>(divisor));
    out_byte(0x40, static_cast<uint8_t>(divisor >> 8));

    interrupt::register_irq_handler(0, timer_handler);
}
