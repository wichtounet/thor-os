//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <stdarg.h>

#include <types.hpp>
#include <string.hpp>

#include "assert.hpp"
#include "console.hpp"
#include "vesa.hpp"
#include "logging.hpp"

#include "text_console.hpp"
#include "vesa_console.hpp"

namespace {

text_console t_console;
vesa_console v_console;
bool text = true;

} //end of anonymous namespace

void stdio::init_console() {
    text = !vesa::enabled();

    if (text) {
        t_console.init();
    } else {
        v_console.init();
    }
}

void stdio::console::init() {
    if (!text) {
        buffer = vesa::create_buffer();
    }
}

size_t stdio::console::get_rows() const {
    if (text) {
        return t_console.lines();
    } else {
        return v_console.lines();
    }
}

size_t stdio::console::get_columns() const {
    if (text) {
        return t_console.columns();
    } else {
        return v_console.columns();
    }
}

void stdio::console::print(char key) {
    if (key == '\n') {
        next_line();
    } else if (key == '\r') {
        // Ignore \r for now
    } else if (key == '\b') {
        --current_column;
        print(' ');
        --current_column;
    } else if (key == '\t') {
        print(' ');
        print(' ');
    } else {
        if (text) {
            t_console.print_char(current_line, current_column, key);
        } else {
            if (active) {
                v_console.print_char(current_line, current_column, key);
            } else {
                v_console.print_char(buffer, current_line, current_column, key);
            }
        }

        ++current_column;

        if (current_column == console::get_columns()) {
            next_line();
        }
    }
}

void stdio::console::wipeout() {
    if (text) {
        t_console.clear();
    } else {
        if (active) {
            v_console.clear();
        } else {
            v_console.clear(buffer);
        }
    }

    current_line   = 0;
    current_column = 0;
}

void stdio::console::save() {
    thor_assert(!text, "save/restore of the text console is not yet supported");

    buffer = v_console.save(buffer);
}

void stdio::console::restore() {
    thor_assert(!text, "save/restore of the text console is not yet supported");

    v_console.restore(buffer);
}

void stdio::console::set_active(bool active) {
    this->active = active;
}

void stdio::console::next_line() {
    ++current_line;

    if (current_line == console::get_rows()) {
        if (text) {
            t_console.scroll_up();
        } else {
            if (active) {
                v_console.scroll_up();
            } else {
                v_console.scroll_up(buffer);
            }
        }

        --current_line;
    }

    current_column = 0;
}
