//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "kernel.hpp"
#include "memory.hpp"
#include "timer.hpp"
#include "shell.hpp"
#include "keyboard.hpp"
#include "disks.hpp"
#include "acpi.hpp"
#include "interrupts.hpp"
#include "arch.hpp"
#include "e820.hpp"

extern "C" {

void _init();

void  kernel_main(){
    arch::enable_sse();

    interrupt::install_idt();
    interrupt::install_isrs();
    interrupt::remap_irqs();
    interrupt::install_irqs();
    interrupt::enable_interrupts();

    e820::finalize_memory_detection();

    init_memory_manager();
    install_timer();
    //acpi::init();
    keyboard::install_driver();
    disks::detect_disks();

    //Call global cosntructors
    _init();

    //Launch the shell
    init_shell();

    return;
}

}
