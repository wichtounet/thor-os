//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TEXT_CONSOLE_H
#define TEXT_CONSOLE_H

#include "stl/types.hpp"

struct text_console {
    void init();
    size_t lines();
    size_t columns();
    void clear();
    void scroll_up();
    void print_char(size_t line, size_t column, char c);
};

#endif
