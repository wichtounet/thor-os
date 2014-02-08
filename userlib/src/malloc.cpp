//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "malloc.hpp"

void* malloc(size_t size){
    //TODO

    return nullptr;
}

void free(void* pointer){
    //TODO
}

size_t brk_start(){
    size_t value;
    asm volatile("mov rax, 7; int 50; mov %0, rax"
        : "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t brk_end(){
    size_t value;
    asm volatile("mov rax, 8; int 50; mov %0, rax"
        : "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t sbrk(size_t inc){
    size_t value;
    asm volatile("mov rax, 9; int 50; mov %0, rax"
        : "=m" (value)
        : "b" (inc)
        : "rax");
    return value;
}
