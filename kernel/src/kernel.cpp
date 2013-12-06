//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "memory.hpp"
#include "timer.hpp"
#include "shell.hpp"
#include "keyboard.hpp"
#include "disks.hpp"
#include "acpi.hpp"

extern "C" {

void _init();

void  __attribute__ ((section ("main_section"))) kernel_main(){
    load_memory_map();
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
