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
#include "arch.hpp"
#include "e820.hpp"
#include "vesa.hpp"
#include "console.hpp"

extern "C" {

void _init();

void  kernel_main(){
    arch::enable_sse();

    interrupt::setup_interrupts();

    e820::finalize_memory_detection();

    init_memory_manager();

    install_timer();
    //acpi::init();
    keyboard::install_driver();
    disks::detect_disks();
    vesa::init();

    //Call global constructors
    _init();

    init_console();

    //Launch the shell
    init_shell();

    return;
}

}
