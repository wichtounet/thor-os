//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_PRINT_HPP
#define USER_PRINT_HPP

//TODO Rename in console

#include <types.hpp>

void print(char c){
    asm volatile("mov rax, 0; mov rbx, %0; int 50"
        : //No outputs
        : "r" (static_cast<size_t>(c))
        : "rax", "rbx");
}

void print(const char* s){
    asm volatile("mov rax, 1; mov rbx, %0; int 50"
        : //No outputs
        : "r" (reinterpret_cast<size_t>(s))
        : "rax", "rbx");
}

void print(size_t v){
    asm volatile("mov rax, 2; mov rbx, %0; int 50"
        : //No outputs
        : "r" (v)
        : "rax", "rbx");
}

void print_line(){
    print('\n');
}

void print_line(const char* s){
    print(s);
    print_line();
}

void print_line(size_t v){
    print(v);
    print_line();
}

char read_char(){
    size_t value;
    asm volatile("mov rax, 3; int 50; mov %0, rax"
        : "=m" (value)
        : //No inputs
        : "rax", "rbx");
    return value;
}

#endif
