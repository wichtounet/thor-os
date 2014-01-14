//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONSOLE_H
#define CONSOLE_H

#include "stl/types.hpp"
#include "stl/enable_if.hpp"
#include "stl/string.hpp"

void init_console();

void set_column(long column);
long get_column();

void set_line(long line);
long get_line();

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

void k_printf(const char* fmt, ...);

template<typename... Arguments>
typename std::enable_if<(sizeof...(Arguments) == 0), void>::type k_print_line(const Arguments&... args){
    k_print('\n');
}

template<typename... Arguments>
typename std::enable_if<(sizeof...(Arguments) > 0), void>::type k_print_line(const Arguments&... args){
    k_print(args...);
    k_print('\n');
}

#endif
