//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TERMINAL_H
#define TERMINAL_H

#include <types.hpp>
#include <circular_buffer.hpp>

#include "sleep_queue.hpp"

namespace stdio {

constexpr const size_t INPUT_BUFFER_SIZE = 256;

struct virtual_terminal {
    size_t id;
    bool active;
    bool canonical;

    circular_buffer<char, INPUT_BUFFER_SIZE> input_buffer;
    circular_buffer<char, 2 * INPUT_BUFFER_SIZE> canonical_buffer;
    circular_buffer<size_t, 3 * INPUT_BUFFER_SIZE> raw_buffer;

    sleep_queue input_queue;

    void print(char c);
    void send_input(char c);

    /*!
     * \brief Reads canonical input in the given buffer
     * \param buffer The buffer to fill
     * \param max The max number of characters to read
     * \return The number of characters that have been read
     */
    size_t read_input_can(char* buffer, size_t max);

    /*!
     * \brief Reads non-canonical input in the given buffer
     * \return the keyboard scan code
     */
    size_t read_input_raw();

    void set_canonical(bool can);

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
