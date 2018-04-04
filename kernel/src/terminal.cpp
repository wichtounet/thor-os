//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <array.hpp>

#include "drivers/keyboard.hpp"

#include "terminal.hpp"
#include "console.hpp"
#include "assert.hpp"
#include "logging.hpp"
#include "scheduler.hpp"

#include "print.hpp"

void stdio::virtual_terminal::print(char key) {
    cons.print(key);
}

void stdio::virtual_terminal::send_input(char key) {
    if (!input_thread_pid) {
        return;
    }

    // Simply give the input to the input thread
    keyboard_buffer.push(key);
    thor_assert(!keyboard_buffer.full(), "keyboard buffer is full!");

    // Need hint here because it is coming from an IRQ
    scheduler::unblock_process_hint(input_thread_pid);
}

void stdio::virtual_terminal::send_mouse_input(std::keycode key) {
    if (!input_thread_pid) {
        return;
    }

    // Simply give the input to the input thread
    mouse_buffer.push(size_t(key));
    thor_assert(!mouse_buffer.full(), "mouse buffer is full!");

    // Need hint here because it is coming from an IRQ
    scheduler::unblock_process_hint(input_thread_pid);
}

size_t stdio::virtual_terminal::read_input_can(char* buffer, size_t max) {
    size_t read = 0;

    while (true) {
        while (!canonical_buffer.empty()) {
            auto c = canonical_buffer.pop();

            buffer[read++] = c;

            if (read >= max || c == '\n') {
                return read;
            }
        }

        input_queue.wait();
    }
}

// TODO In case of max < read, the timeout is not respected
size_t stdio::virtual_terminal::read_input_can(char* buffer, size_t max, size_t ms) {
    size_t read = 0;

    while (true) {
        while (!canonical_buffer.empty()) {
            auto c = canonical_buffer.pop();

            buffer[read++] = c;

            if (read >= max || c == '\n') {
                return read;
            }
        }

        if (!ms) {
            return read;
        }

        if (!input_queue.wait_for(ms)) {
            return read;
        }
    }
}

size_t stdio::virtual_terminal::read_input_raw() {
    if (raw_buffer.empty()) {
        input_queue.wait();
    }

    return raw_buffer.pop();
}

size_t stdio::virtual_terminal::read_input_raw(size_t ms) {
    if (raw_buffer.empty()) {
        if (!ms) {
            return static_cast<size_t>(std::keycode::TIMEOUT);
        }

        if (!input_queue.wait_for(ms)) {
            return static_cast<size_t>(std::keycode::TIMEOUT);
        }
    }

    thor_assert(!raw_buffer.empty(), "There is a problem with the sleep queue: Nothing to read from raw_buffer");

    return raw_buffer.pop();
}

void stdio::virtual_terminal::set_canonical(bool can) {
    logging::logf(logging::log_level::TRACE, "Switched terminal %u canonical mode from %u to %u\n", id, uint64_t(canonical), uint64_t(can));

    canonical = can;
}

void stdio::virtual_terminal::set_mouse(bool m) {
    logging::logf(logging::log_level::TRACE, "Switched terminal %u mouse handling mode from %u to %u\n", id, uint64_t(mouse), uint64_t(m));

    mouse = m;
}

bool stdio::virtual_terminal::is_canonical() const {
    return canonical;
}

bool stdio::virtual_terminal::is_mouse() const {
    return mouse;
}

void stdio::virtual_terminal::set_active(bool active) {
    if (this->active == active) {
        return;
    }

    this->active = active;

    cons.set_active(active);

    if (active) {
        cons.restore();
    } else {
        cons.save();
    }
}

stdio::console& stdio::virtual_terminal::get_console() {
    return cons;
}
