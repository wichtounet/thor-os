//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>

#include <types.hpp>
#include <enable_if.hpp>
#include <string.hpp>

namespace console {

void init();

size_t get_columns();
size_t get_rows();

void* save(void* buffer);
void restore(void* buffer);

void set_column(size_t column);
size_t get_column();

void set_line(size_t line);
size_t get_line();

void wipeout();

} // end of namespace console

#endif
