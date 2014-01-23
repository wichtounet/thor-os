//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "kernel.hpp"
#include "physical_allocator.hpp"
#include "virtual_allocator.hpp"
#include "paging.hpp"
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

extern "C" {

void _init();

void  kernel_main(){
    arch::enable_sse();

    gdt::flush_tss();

    interrupt::setup_interrupts();

    //Prepare memory
    physical_allocator::early_init();
    paging::init();
    virtual_allocator::init();

    physical_allocator::init();
    init_memory_manager();

    //Install drivers
    install_timer();
    //acpi::init();
    keyboard::install_driver();
    disks::detect_disks();
    vesa::init();

    //Only install system calls when everything else is ready
    install_system_calls();

    //Call global constructors
    _init();

    init_console();

    //Launch the shell
    init_shell();

    return;
}

}

void suspend_boot(){
    k_print_line("Impossible to continue boot...");
    asm volatile("hlt");
    __builtin_unreachable();
}

void suspend_kernel(){
    asm volatile("hlt");
    __builtin_unreachable();
}
