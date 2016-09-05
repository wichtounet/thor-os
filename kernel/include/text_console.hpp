//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TEXT_CONSOLE_H
#define TEXT_CONSOLE_H

#include <types.hpp>

struct text_console {
    void init();
    size_t lines();
    size_t columns();
    void clear();
    void scroll_up();
    void print_char(size_t line, size_t column, char c);
};

#endif
