//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "system_calls.hpp"
#include "console.hpp"
#include "scheduler.hpp"
#include "keyboard.hpp"

namespace {

void sc_print_char(const interrupt::syscall_regs& regs){
    k_print(static_cast<char>(regs.rbx));
}

void sc_print_string(const interrupt::syscall_regs& regs){
    k_print(reinterpret_cast<const char*>(regs.rbx));
}

void sc_print_digit(const interrupt::syscall_regs& regs){
    k_print(regs.rbx);
}

void sc_get_char(const interrupt::syscall_regs& regs){
    auto c = keyboard::get_char_blocking();
    //TODO
}

} //End of anonymous namespace

void system_call_entry(const interrupt::syscall_regs& regs){
    auto code = regs.rax;

    switch(code){
        case 0:
            sc_print_char(regs);
            break;

        case 1:
            sc_print_string(regs);
            break;

        case 2:
            sc_print_digit(regs);
            break;

        case 3:
            sc_get_char(regs);
            break;

        case 0x666:
            scheduler::kill_current_process(regs);
            break;

        default:
            k_print_line("Invalid system call");
            break;
    }

    //Reschedule to make sure that BLOCKED process will be preempted
    scheduler::reschedule(regs);
}

void install_system_calls(){
    interrupt::register_syscall_handler(0, &system_call_entry);
}
