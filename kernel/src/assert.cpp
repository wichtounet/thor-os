//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
