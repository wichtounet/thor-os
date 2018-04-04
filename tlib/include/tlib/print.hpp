//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USER_PRINT_HPP
#define USER_PRINT_HPP

#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

#include <stdarg.h>

#include <types.hpp>
#include <string.hpp>

#include <tlib/keycode.hpp>

namespace tlib {

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

void set_canonical(bool can);
void set_mouse(bool m);

size_t read_input(char* buffer, size_t max);
size_t read_input(char* buffer, size_t max, size_t ms);

std::keycode read_input_raw();
std::keycode read_input_raw(size_t ms);

void clear();

size_t get_columns();
size_t get_rows();

#include "printf_dec.hpp"

void user_logf(const char* s, ...);

} //end of namespace tlib

#endif
