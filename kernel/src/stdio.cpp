//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <array.hpp>

#include "drivers/keyboard.hpp"

#include "stdio.hpp"
#include "terminal.hpp"
#include "terminal_driver.hpp"
#include "console.hpp"
#include "assert.hpp"
#include "logging.hpp"
#include "scheduler.hpp"
#include "physical_allocator.hpp"

namespace {

stdio::terminal_driver terminal_driver_impl;
stdio::terminal_driver* tty_driver = &terminal_driver_impl;

constexpr const size_t MAX_TERMINALS = 3;

size_t _terminals_count;
size_t active_terminal;

std::array<stdio::virtual_terminal, MAX_TERMINALS> terminals;

void input_thread(void* data) {
    auto& terminal = *reinterpret_cast<stdio::virtual_terminal*>(data);

    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "stdio: Input Thread for terminal %u started (pid:%u)\n", terminal.id, pid);

    bool shift = false;
    bool alt   = false;

    while (true) {
        // Wait for some input
        scheduler::block_process(pid);

        // Handle keyboard input
        while (!terminal.keyboard_buffer.empty()) {
            auto key = terminal.keyboard_buffer.pop();

            if (terminal.canonical) {
                //Key released
                if (key & 0x80) {
                    key &= ~(0x80);

                    if (alt && key == keyboard::KEY_F1) {
                        stdio::switch_terminal(0);
                    } else if (alt && key == keyboard::KEY_F2) {
                        stdio::switch_terminal(1);
                    } else if (alt && key == keyboard::KEY_F3) {
                        stdio::switch_terminal(2);
                    }

                    if (key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT) {
                        shift = false;
                    }

                    if (key == keyboard::KEY_ALT) {
                        alt = false;
                    }
                }
                //Key pressed
                else {
                    if (key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT) {
                        shift = true;
                    } else if (key == keyboard::KEY_ALT) {
                        alt = true;
                    } else if (key == keyboard::KEY_BACKSPACE) {
                        if (!terminal.input_buffer.empty()) {
                            terminal.input_buffer.pop_last();
                            terminal.print('\b');
                        }
                    } else {
                        auto qwertz_key =
                            shift
                                ? keyboard::shift_key_to_ascii(key)
                                : keyboard::key_to_ascii(key);

                        if (qwertz_key) {
                            terminal.input_buffer.push(qwertz_key);

                            terminal.print(qwertz_key);

                            if (qwertz_key == '\n') {
                                // Transfer current line to the canonical buffer
                                while (!terminal.input_buffer.empty()) {
                                    terminal.canonical_buffer.push(terminal.input_buffer.pop());
                                }

                                terminal.input_queue.notify_one();
                            }
                        }
                    }
                }
            } else {
                // The complete processing of the key will be done by the
                // userspace program
                auto code = keyboard::raw_key_to_keycode(key);
                terminal.raw_buffer.push(static_cast<size_t>(code));

                terminal.input_queue.notify_one();

                thor_assert(!terminal.raw_buffer.full(), "raw buffer is full!");
            }
        }

        // Handle mouse input
        while (!terminal.mouse_buffer.empty()) {
            auto key = terminal.mouse_buffer.pop();

            if (!terminal.canonical && terminal.is_mouse()) {
                terminal.raw_buffer.push(key);

                terminal.input_queue.notify_one();

                thor_assert(!terminal.raw_buffer.full(), "raw buffer is full!");
            }
        }
    }
}

} //end of anonymous namespace

void stdio::init_terminals() {
    size_t id = 0;

    if(physical_allocator::available() < 64 * 1024 * 1024){
        _terminals_count = 1;
    } else {
        _terminals_count = MAX_TERMINALS;
    }

    logging::logf(logging::log_level::DEBUG, "stdio: using %u terminals\n", _terminals_count);

    for (size_t i = 0; i < _terminals_count; ++i){
        auto& terminal = terminals[i];

        terminal.id        = id++;
        terminal.active    = false;
        terminal.canonical = true;
        terminal.mouse     = false;
    }

    // Initialize the active terminal

    active_terminal                   = 0;
    terminals[active_terminal].active = true;
    terminals[active_terminal].get_console().set_active(true);
}

void stdio::register_devices() {
    for (size_t i = 0; i < _terminals_count; ++i){
        auto& terminal = terminals[i];

        std::string name = std::string("tty") + std::to_string(terminal.id);

        devfs::register_device("/dev/", name, devfs::device_type::CHAR_DEVICE, tty_driver, &terminal);
    }
}

void stdio::finalize() {
    for (size_t i = 0; i < _terminals_count; ++i){
        auto& terminal = terminals[i];

        auto* user_stack   = new char[scheduler::user_stack_size];
        auto* kernel_stack = new char[scheduler::kernel_stack_size];

        auto& input_process    = scheduler::create_kernel_task_args("tty_input", user_stack, kernel_stack, &input_thread, &terminal);
        input_process.ppid     = 1;
        input_process.priority = scheduler::DEFAULT_PRIORITY;

        scheduler::queue_system_process(input_process.pid);

        terminal.input_thread_pid = input_process.pid;

        // Save the initial image of the terminal
        terminal.get_console().init();
        terminal.get_console().save();
    }
}

size_t stdio::terminals_count() {
    return _terminals_count;
}

stdio::virtual_terminal& stdio::get_active_terminal() {
    return terminals[active_terminal];
}

stdio::virtual_terminal& stdio::get_terminal(size_t id) {
    thor_assert(id < _terminals_count, "Out of bound tty");

    return terminals[id];
}

void stdio::switch_terminal(size_t id) {
    if (active_terminal != id) {
        logging::logf(logging::log_level::TRACE, "stdio: Switch activate virtual terminal %u\n", id);

        // Effectively switch the terminal
        terminals[active_terminal].set_active(false);
        terminals[id].set_active(true);

        active_terminal = id;
    }
}
