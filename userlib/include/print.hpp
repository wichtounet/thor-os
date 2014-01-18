//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_PRINT_HPP
#define USER_PRINT_HPP

#include <types.hpp>

void print(char c){
    asm volatile("mov rax, 0; mov rbx, %0; int 50"
        : //No outputs
        : "r" (static_cast<size_t>(c))
        : "rax", "rbx");
}

#endif
