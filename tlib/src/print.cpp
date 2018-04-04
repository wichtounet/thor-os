//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <stdarg.h>

#include "tlib/print.hpp"
#include "tlib/file.hpp"

namespace {

size_t strlen(const char* c){
    size_t s = 0;

    while(*c++){
        ++s;
    }

    return s;
}

} //end of anonymous namespace

void tlib::print(char c){
    tlib::write(2, &c, 1, 0);
}

void tlib::print(const char* s){
    tlib::write(2, s, strlen(s), 0);
}

void tlib::print(const std::string& s){
    tlib::write(2, s.c_str(), s.size(), 0);
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
    auto c = tlib::read(1, buffer, max, 0);
    if(c){
        return *c;
    } else {
        return 0;
    }
}

size_t tlib::read_input(char* buffer, size_t max, size_t ms){
    auto c = tlib::read(1, buffer, max, 0, ms);
    if(c){
        return *c;
    } else {
        return 0;
    }
}

std::keycode tlib::read_input_raw(){
    char value;
    auto c = tlib::read(1, &value, 1, 0);
    if(c){
        if (*c) {
            return static_cast<std::keycode>(value);
        } else {
            return static_cast<std::keycode>(0);
        }
    } else {
        return static_cast<std::keycode>(0);
    }
}

std::keycode tlib::read_input_raw(size_t ms){
    char value;
    auto c = tlib::read(1, &value, 1, 0, ms);
    if(c){
        if (*c) {
            return static_cast<std::keycode>(value);
        } else {
            return static_cast<std::keycode>(0);
        }
    } else {
        return static_cast<std::keycode>(0);
    }
}

void  tlib::clear(){
    asm volatile("mov rax, 0x22; int 50;"
        : //No outputs
        : //No inputs
        : "rax");
}

size_t tlib::get_columns(){
    size_t value;
    asm volatile("mov rax, 0x23; int 50; mov %[columns], rax"
        : [columns] "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t tlib::get_rows(){
    size_t value;
    asm volatile("mov rax, 0x24; int 50; mov %[rows], rax"
        : [rows] "=m" (value)
        : //No inputs
        : "rax");
    return value;
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
