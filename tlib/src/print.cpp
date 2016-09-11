//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <stdarg.h>

#include "print.hpp"

void print(char c){
    asm volatile("mov rax, 0; mov rbx, %[c]; int 50"
        : //No outputs
        : [c] "g" (static_cast<size_t>(c))
        : "rax", "rbx");
}

void print(const char* s){
    asm volatile("mov rax, 1; mov rbx, %[s]; int 50"
        : //No outputs
        : [s] "g" (reinterpret_cast<size_t>(s))
        : "rax", "rbx");
}

void print(uint8_t v){
    print(std::to_string(v));
}

void print(uint16_t v){
    print(std::to_string(v));
}

void print(uint32_t v){
    print(std::to_string(v));
}

void print(uint64_t v){
    print(std::to_string(v));
}

void print(int8_t v){
    print(std::to_string(v));
}

void print(int16_t v){
    print(std::to_string(v));
}

void print(int32_t v){
    print(std::to_string(v));
}

void print(int64_t v){
    print(std::to_string(v));
}

size_t read_input(char* buffer, size_t max){
    size_t value;
    asm volatile("mov rax, 3; mov rbx, %[buffer]; mov rcx, %[max]; int 50; mov %[read], rax"
        : [read] "=m" (value)
        : [buffer] "g" (buffer), [max] "g" (max)
        : "rax", "rbx", "rcx");
    return value;
}

void  clear(){
    asm volatile("mov rax, 100; int 50;"
        : //No outputs
        : //No inputs
        : "rax");
}

size_t get_columns(){
    size_t value;
    asm volatile("mov rax, 101; int 50; mov %[columns], rax"
        : [columns] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t get_rows(){
    size_t value;
    asm volatile("mov rax, 102; int 50; mov %[rows], rax"
        : [rows] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

void print(const std::string& s){
    return print(s.c_str());
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

void print_line(const std::string& s){
    print(s);
    print_line();
}

#include "printf_def.hpp"

void __printf(const std::string& formatted){
    print(formatted);
}

void __printf_raw(const char* formatted){
    print(formatted);
}
