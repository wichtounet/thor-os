//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_PRINT_HPP
#define USER_PRINT_HPP

//TODO Rename in console

#include <stdarg.h>

#include <types.hpp>
#include <string.hpp>

void print(char c);
void print(const char* s);
void print(const std::string& s);

void print(uint8_t v);
void print(uint16_t v);
void print(uint32_t v);
void print(uint64_t v);

void print(int8_t v);
void print(int16_t v);
void print(int32_t v);
void print(int64_t v);

void print_line();
void print_line(const char* s);
void print_line(size_t v);
void print_line(const std::string& s);

size_t read_input(char* buffer, size_t max);

void clear();

size_t get_columns();
size_t get_rows();

#include "printf_dec.hpp"

#endif
