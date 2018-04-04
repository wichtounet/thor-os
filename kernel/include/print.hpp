//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>

#include <types.hpp>
#include <enable_if.hpp>
#include <string.hpp>

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
