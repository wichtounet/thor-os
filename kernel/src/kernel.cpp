//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
#include "keyboard.hpp"
#include "serial.hpp"
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
#include "logging.hpp"
#include "vfs/vfs.hpp"
#include "fs/sysfs.hpp"

extern "C" {

void __cxa_pure_virtual(){
    k_print_line("A pure virtual function has been called");
    suspend_kernel();
}

void _init();

void  kernel_main(){
    //Make sure stack is aligned to 16 byte boundary
    asm volatile("and rsp, -16");

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

    //Try to init VESA
    if(vesa::vesa_enabled && !vesa::init()){
        vesa::vesa_enabled = false;

        //Unfortunately, we are in long mode, we cannot go back
        //to text mode for now
        suspend_boot();
    }

    init_console();
    stdio::init_terminals();

    //Starting from here, the logging system can use the console
    logging::finalize();

    //Finalize memory operations (register sysfs values)
    paging::finalize();
    physical_allocator::finalize();
    virtual_allocator::finalize();
    kalloc::finalize();

    //Install drivers
    serial::init();
    timer::install();
    //acpi::init();
    keyboard::install_driver();
    disks::detect_disks();

    //Init the virtual file system
    vfs::init();

    //Starting from here, the logging system can output logs to file
    //TODO logging::to_file();

    //Only install system calls when everything else is ready
    install_system_calls();

    sysfs::set_constant_value("/sys/", "version", "0.1");
    sysfs::set_constant_value("/sys/", "author", "Baptiste Wicht");

    scheduler::init();
    scheduler::start();
}

}

void suspend_boot(){
    k_print_line("Impossible to continue boot...");
    suspend_kernel();
}

void suspend_kernel(){
    asm volatile("cli; hlt");
    __builtin_unreachable();
}
