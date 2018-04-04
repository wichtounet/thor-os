//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "terminal_driver.hpp"
#include "terminal.hpp"

size_t stdio::terminal_driver::read(void* data, char* buffer, size_t count, size_t& read){
    auto* terminal = reinterpret_cast<stdio::virtual_terminal*>(data);

    if (terminal->is_canonical()) {
        read = terminal->read_input_can(reinterpret_cast<char*>(buffer), count);
    } else {
        buffer[0] = terminal->read_input_raw();

        read = 1;
    }

    return 0;
}

size_t stdio::terminal_driver::read(void* data, char* buffer, size_t count, size_t& read, size_t ms){
    auto* terminal = reinterpret_cast<stdio::virtual_terminal*>(data);

    if(terminal->is_canonical()){
        read = terminal->read_input_can(reinterpret_cast<char*>(buffer), count, ms);
    } else {
        buffer[0] = terminal->read_input_raw(ms);

        read = 1;
    }

    return 0;
}

size_t stdio::terminal_driver::write(void* data, const char* buffer, size_t count, size_t& written){
    auto* terminal = reinterpret_cast<stdio::virtual_terminal*>(data);

    for(size_t i = 0; i < count;++i){
        terminal->print(buffer[i]);
    }

    written = count;

    return 0;
}
