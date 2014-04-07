//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>

#include <types.hpp>
#include <enable_if.hpp>
#include <string.hpp>

void init_console();

size_t get_columns();
size_t get_rows();

void wipeout();
void k_print(char key);
void k_print(const char* string);
void k_print(const char* string, uint64_t end);

void k_print(const std::string& s);

void k_print(uint8_t number);
void k_print(uint16_t number);
void k_print(uint32_t number);
void k_print(uint64_t number);

void k_print(int8_t number);
void k_print(int16_t number);
void k_print(int32_t number);
void k_print(int64_t number);

template<typename... Arguments>
typename std::enable_if_t<(sizeof...(Arguments) == 0)> k_print_line(const Arguments&... args){
    k_print('\n');
}

template<typename... Arguments>
typename std::enable_if_t<(sizeof...(Arguments) > 0)> k_print_line(const Arguments&... args){
    k_print(args...);
    k_print('\n');
}

#include "printf_dec.hpp"

#endif
