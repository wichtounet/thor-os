//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TERMINAL_H
#define TERMINAL_H

#include <types.hpp>
#include <array.hpp>
#include <circular_buffer.hpp>

#include "sleep_queue.hpp"
#include "mutex.hpp"

namespace stdio {

constexpr const size_t INPUT_BUFFER_SIZE = 128;

struct virtual_terminal {
    size_t id;
    bool active;
    bool canonical;

    volatile size_t first;
    volatile size_t last;

    std::array<char, INPUT_BUFFER_SIZE> buffer_canonical;

    mutex lock;
    sleep_queue input_queue;
    
    void send_input(char c);
    void handle_input(char c);

    void print(char c);
    size_t read_input(char* buffer, size_t max);

    virtual_terminal(){}

    virtual_terminal(const virtual_terminal& rhs) = delete;
    virtual_terminal& operator=(const virtual_terminal& rhs) = delete;

    virtual_terminal(virtual_terminal&& rhs) = delete;
    virtual_terminal& operator=(virtual_terminal&& rhs) = delete;
};

void init_terminals();
virtual_terminal& get_active_terminal();
virtual_terminal& get_terminal(size_t id);

} //end of namespace stdio

#endif
