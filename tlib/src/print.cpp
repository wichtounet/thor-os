//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <stdarg.h>

#include "tlib/print.hpp"

void tlib::print(char c){
    asm volatile("mov rax, 0; mov rbx, %[c]; int 50"
        : //No outputs
        : [c] "g" (static_cast<size_t>(c))
        : "rax", "rbx");
}

void tlib::print(const char* s){
    asm volatile("mov rax, 1; mov rbx, %[s]; int 50"
        : //No outputs
        : [s] "g" (reinterpret_cast<size_t>(s))
        : "rax", "rbx");
}

void log(const char* s){
    asm volatile("mov rax, 2; mov rbx, %[s]; int 50"
        : //No outputs
        : [s] "g" (reinterpret_cast<size_t>(s))
        : "rax", "rbx");
}

void tlib::print(uint8_t v){
    print(std::to_string(v));
}

void tlib::print(uint16_t v){
    print(std::to_string(v));
}

void tlib::print(uint32_t v){
    print(std::to_string(v));
}

void tlib::print(uint64_t v){
    print(std::to_string(v));
}

void tlib::print(int8_t v){
    print(std::to_string(v));
}

void tlib::print(int16_t v){
    print(std::to_string(v));
}

void tlib::print(int32_t v){
    print(std::to_string(v));
}

void tlib::print(int64_t v){
    print(std::to_string(v));
}

void tlib::set_canonical(bool can){
    size_t value = can;
    asm volatile("mov rax, 0x20; mov rbx, %[value]; int 50;"
        :
        : [value] "g" (value)
        : "rax", "rbx");
}

void tlib::set_mouse(bool m){
    size_t value = m;
    asm volatile("mov rax, 0x21; mov rbx, %[value]; int 50;"
        :
        : [value] "g" (value)
        : "rax", "rbx");
}

size_t tlib::read_input(char* buffer, size_t max){
    size_t value;
    asm volatile("mov rax, 0x10; mov rbx, %[buffer]; mov rcx, %[max]; int 50; mov %[read], rax"
        : [read] "=m" (value)
        : [buffer] "g" (buffer), [max] "g" (max)
        : "rax", "rbx", "rcx");
    return value;
}

size_t tlib::read_input(char* buffer, size_t max, size_t ms){
    size_t value;
    asm volatile("mov rax, 0x11; mov rbx, %[buffer]; mov rcx, %[max]; mov rdx, %[ms]; int 50; mov %[read], rax"
        : [read] "=m" (value)
        : [buffer] "g" (buffer), [max] "g" (max), [ms] "g" (ms)
        : "rax", "rbx", "rcx");
    return value;
}

keycode tlib::read_input_raw(){
    size_t value;
    asm volatile("mov rax, 0x12; int 50; mov %[input], rax"
        : [input] "=m" (value)
        :
        : "rax");
    return static_cast<keycode>(value);
}

keycode tlib::read_input_raw(size_t ms){
    size_t value;
    asm volatile("mov rax, 0x13; mov rbx, %[ms]; int 50; mov %[input], rax"
        : [input] "=m" (value)
        : [ms] "g" (ms)
        : "rax");
    return static_cast<keycode>(value);
}

void  tlib::clear(){
    asm volatile("mov rax, 100; int 50;"
        : //No outputs
        : //No inputs
        : "rax");
}

size_t tlib::get_columns(){
    size_t value;
    asm volatile("mov rax, 101; int 50; mov %[columns], rax"
        : [columns] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t tlib::get_rows(){
    size_t value;
    asm volatile("mov rax, 102; int 50; mov %[rows], rax"
        : [rows] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

void tlib::print(const std::string& s){
    return print(s.c_str());
}

void tlib::print_line(){
    print('\n');
}

void tlib::print_line(const char* s){
    print(s);
    print_line();
}

void tlib::print_line(size_t v){
    print(v);
    print_line();
}

void tlib::print_line(const std::string& s){
    print(s);
    print_line();
}

void tlib::user_logf(const char* s, ...){
    va_list va;
    va_start(va, s);

    char buffer[512];
    vsprintf_raw(buffer, 512, s, va);
    log(buffer);

    va_end(va);
}

namespace tlib {

#include "printf_def.hpp"

void __printf(const std::string& formatted){
    print(formatted);
}

void __printf_raw(const char* formatted){
    print(formatted);
}

} // end of namespace tlib
