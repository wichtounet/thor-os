//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include <types.hpp>

void exit(size_t return_code) __attribute__((noreturn));

void exit(size_t return_code) {
    asm volatile("mov rax, 0x666; int 50"
        : //No outputs
        : "b" (return_code)
        : "rax");

    __builtin_unreachable();
}

void sleep_ms(size_t ms){
    asm volatile("mov rax, 4; int 50"
        : //No outputs
        : "b" (ms)
        : "rax");
}

#endif
