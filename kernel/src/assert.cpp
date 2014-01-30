//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "console.hpp"
#include "kernel.hpp"

void __thor_assert(bool condition){
    __thor_assert(condition, "assertion failed");
}

void __thor_assert(bool condition, const char* message){
    if(!condition){
        k_print_line(message);
        suspend_kernel();
    }
}

void __thor_unreachable(const char* message){
    k_print_line(message);
    suspend_kernel();
}
