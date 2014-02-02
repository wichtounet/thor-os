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
#include "terminal.hpp"

namespace {

void sc_print_char(interrupt::syscall_regs* regs){
    k_print(static_cast<char>(regs->rbx));
}

void sc_print_string(interrupt::syscall_regs* regs){
    k_print(reinterpret_cast<const char*>(regs->rbx));
}

void sc_print_digit(interrupt::syscall_regs* regs){
    k_print(regs->rbx);
}

void sc_get_input(interrupt::syscall_regs* regs){
    auto ttyid = scheduler::get_process(scheduler::get_pid()).tty;
    auto& tty = stdio::get_terminal(ttyid);

    regs->rax = tty.read_input(reinterpret_cast<char*>(regs->rbx), regs->rcx);
}

} //End of anonymous namespace

void system_call_entry(interrupt::syscall_regs* regs){
    auto code = regs->rax;

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
            sc_get_input(regs);
            break;

        case 0x666:
            scheduler::kill_current_process();
            break;

        default:
            k_print_line("Invalid system call");
            break;
    }

    //TODO Perhaps not interesting anymore
    //Reschedule to make sure that BLOCKED process will be preempted
    scheduler::reschedule();
}

void install_system_calls(){
    interrupt::register_syscall_handler(0, &system_call_entry);
}
