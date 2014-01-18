//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "system_calls.hpp"
#include "console.hpp"

void system_call_entry(const interrupt::syscall_regs& regs){
    k_print_line("system_call");
}

void install_system_calls(){
    interrupt::register_syscall_handler(0, &system_call_entry);
}
