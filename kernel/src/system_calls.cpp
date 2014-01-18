//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "system_calls.hpp"
#include "console.hpp"

namespace {

void sc_print_char(const interrupt::syscall_regs& regs){
    k_print(static_cast<char>(regs.rbx));
}

} //End of anonymous namespace

void system_call_entry(const interrupt::syscall_regs& regs){
    auto code = regs.rax;

    switch(code){
        case 0:
            sc_print_char(regs);
            break;

        default:
            k_print_line("Invalid system call");
            break;
    }
}

void install_system_calls(){
    interrupt::register_syscall_handler(0, &system_call_entry);
}
