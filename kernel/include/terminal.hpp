//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TERMINAL_H
#define TERMINAL_H

#include <types.hpp>
#include <circular_buffer.hpp>

#include <tlib/keycode.hpp>

#include "conc/condition_variable.hpp"

#include "fs/devfs.hpp"

#include "console.hpp"

namespace stdio {

constexpr const size_t INPUT_BUFFER_SIZE = 256;

/*!
 * \brief A virtual terminal
 */
struct virtual_terminal {
    /*!
     * \brief Construct a new virtual_terminal
     */
    virtual_terminal(){}

    virtual_terminal(const virtual_terminal& rhs) = delete;
    virtual_terminal& operator=(const virtual_terminal& rhs) = delete;

    virtual_terminal(virtual_terminal&& rhs) = delete;
    virtual_terminal& operator=(virtual_terminal&& rhs) = delete;

    /*!
     * \brief Print the given char to the terminal
     * \param c The character to print
     */
    void print(char c);

    /*!
     * \brief Send a keyboard input to the terminal (from the keyboard driver)
     */
    void send_input(char c);

    /*!
     * \brief Send a mouse input to the terminal (from the keyboard driver)
     */
    void send_mouse_input(std::keycode c);

    /*!
     * \brief Reads canonical input in the given buffer
     * \param buffer The buffer to fill
     * \param max The max number of characters to read
     * \return The number of characters that have been read
     */
    size_t read_input_can(char* buffer, size_t max);

    /*!
     * \brief Reads canonical input in the given buffer, with a timeout
     *
     * \param buffer The buffer to fill
     * \param max The max number of characters to read
     * \param ms The maximum time (in ms) to wait
     *
     * \return The number of characters that have been read
     */
    size_t read_input_can(char* buffer, size_t max, size_t ms);

    /*!
     * \brief Reads non-canonical input in the given buffer
     * \return the keyboard key code
     */
    size_t read_input_raw();

    /*!
     * \brief Reads non-canonical input in the given buffer, with
     * a timeout
     * \return the keyboard key code
     */
    size_t read_input_raw(size_t ms);

    /*!
     * \brief Set the canonical mode of the terminal
     * \param can The canonical mode of the terminal
     */
    void set_canonical(bool can);

    /*!
     * \brief Set the mouse mode of the terminal
     * \param can The mouse mode of the terminal
     */
    void set_mouse(bool m);

    /*!
     * \brief Returns true if the terminal is in canonical mode, false otherwise
     */
    bool is_canonical() const;

    /*!
     * \brief Returns true if the terminal is in mouse mode, false otherwise
     */
    bool is_mouse() const;

    /*!
     * \brief Set the active mode of the terminal
     */
    void set_active(bool);

    /*!
     * \brief Returns the console linked to this terminal
     */
    console& get_console();

    size_t id;
    bool active;
    bool canonical;
    bool mouse;
    size_t input_thread_pid;

    // Filled by the IRQ
    circular_buffer<char, 128> keyboard_buffer;
    circular_buffer<size_t, 128> mouse_buffer;

    // Handled by the input thread
    circular_buffer<char, INPUT_BUFFER_SIZE> input_buffer;
    circular_buffer<char, 2 * INPUT_BUFFER_SIZE> canonical_buffer;
    circular_buffer<size_t, 3 * INPUT_BUFFER_SIZE> raw_buffer;

    condition_variable input_queue;

private:
    console cons;
};

} //end of namespace stdio

#endif
