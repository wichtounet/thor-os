//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
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
#include "system_calls.hpp"
#include "arch.hpp"
#include "vesa.hpp"
#include "console.hpp"
#include "gdt.hpp"
#include "physical_allocator.hpp"

extern "C" {

void _init();

void  kernel_main(){
    arch::enable_sse();

    gdt::flush_tss();

    interrupt::setup_interrupts();

    init_physical_allocator();
    init_memory_manager();

    install_timer();
    //acpi::init();
    keyboard::install_driver();
    disks::detect_disks();
    vesa::init();

    install_system_calls();

    //Call global constructors
    _init();

    init_console();

    //Launch the shell
    init_shell();

    return;
}

}
