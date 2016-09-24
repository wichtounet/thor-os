//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VESA_CONSOLE_H
#define VESA_CONSOLE_H

#include <types.hpp>

struct vesa_console {
    void init();

    size_t lines() const ;
    size_t columns() const ;

    void clear();
    void clear(void* buffer);

    void scroll_up();
    void scroll_up(void* buffer);

    void print_char(size_t line, size_t column, char c);
    void print_char(void* buffer, size_t line, size_t column, char c);

    void* save(void* buffer);
    void restore(void* buffer);
};

#endif
