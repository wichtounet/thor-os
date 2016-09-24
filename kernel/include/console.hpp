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

namespace stdio {

void init_console();

struct console {
    size_t get_columns() const ;
    size_t get_rows() const ;

    void print(char c);

    void wipeout();

    void save();
    void restore();

private:
    void next_line();

    size_t current_line = 0;
    size_t current_column = 0;

    void* buffer;
};

} // end of namespace stdio

#endif
