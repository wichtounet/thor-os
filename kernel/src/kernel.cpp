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
#include "kalloc.hpp"
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
#include "terminal.hpp"
#include "scheduler.hpp"
#include "vfs/vfs.hpp"
#include "fs/sysfs.hpp"

extern "C" {

void __cxa_pure_virtual(){
    //TODO Print something
}

void _init();

void  kernel_main(){
    arch::enable_sse();

    gdt::flush_tss();

    interrupt::setup_interrupts();

    //Init the virtual allocator
    virtual_allocator::init();

    //Prepare basic physical allocator for paging init
    physical_allocator::early_init();

    //Init all the physical
    paging::init();

    //Finalize physical allocator initialization for kalloc
    physical_allocator::init();

    //Init dynamic memory allocation
    kalloc::init();

    //Call global constructors
    _init();

    //Finalize memory operations
    paging::finalize();
    physical_allocator::finalize();
    virtual_allocator::finalize();
    kalloc::finalize();

    //Install drivers
    timer::install();
    //acpi::init();
    keyboard::install_driver();
    disks::detect_disks();

    //Init the virtual file system
    vfs::init();

    //Try to init VESA
    if(vesa::vesa_enabled && !vesa::init()){
        vesa::vesa_enabled = false;

        //Unfortunately, we are in long mode, we cannot go back
        //to text mode for now
        suspend_boot();
    }

    stdio::init_terminals();

    //Only install system calls when everything else is ready
    install_system_calls();

    init_console();

    sysfs::set_constant_value("/sys/", "version", "0.1");
    sysfs::set_constant_value("/sys/", "author", "Baptiste Wicht");

    scheduler::init();
    scheduler::start();
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
