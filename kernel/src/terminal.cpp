//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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

namespace {

stdio::terminal_driver terminal_driver_impl;
stdio::terminal_driver* tty_driver = &terminal_driver_impl;

constexpr const size_t MAX_TERMINALS = 2;
size_t active_terminal;

std::array<stdio::virtual_terminal, MAX_TERMINALS> terminals;

void input_thread(void* data){
    auto& terminal = *reinterpret_cast<stdio::virtual_terminal*>(data);

    auto pid = scheduler::get_pid();

    logging::logf(logging::log_level::TRACE, "stdio: Input Thread for terminal %u started (pid:%u)\n", terminal.id, pid);

    bool shift = false;
    bool alt = false;

    while(true){
        // Wait for some input
        scheduler::block_process(pid);

        // Handle keyboard input
        while(!terminal.keyboard_buffer.empty()){
            auto key = terminal.keyboard_buffer.pop();

            if(terminal.canonical){
                //Key released
                if(key & 0x80){
                    key &= ~(0x80);

                    if(alt && key == keyboard::KEY_F1){
                        stdio::switch_terminal(0);
                    } else if(alt && key == keyboard::KEY_F2){
                        stdio::switch_terminal(1);
                    }

                    if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                        shift = false;
                    }

                    if(key == keyboard::KEY_ALT){
                        alt = false;
                    }
                }
                //Key pressed
                else {
                    if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                        shift = true;
                    } else if(key == keyboard::KEY_ALT){
                        alt = true;
                    } else if(key == keyboard::KEY_BACKSPACE){
                        if(!terminal.input_buffer.empty()){
                            terminal.input_buffer.pop_last();
                            terminal.print('\b');
                        }
                    } else {
                        auto qwertz_key =
                            shift
                            ? keyboard::shift_key_to_ascii(key)
                            : keyboard::key_to_ascii(key);

                        if(qwertz_key){
                            terminal.input_buffer.push(qwertz_key);

                            terminal.print(qwertz_key);

                            if(qwertz_key == '\n'){
                                // Transfer current line to the canonical buffer
                                while(!terminal.input_buffer.empty()){
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
        while(!terminal.mouse_buffer.empty()){
            auto key = terminal.mouse_buffer.pop();

            if(!terminal.canonical && terminal.mouse){
                terminal.raw_buffer.push(key);

                terminal.input_queue.notify_one();

                thor_assert(!terminal.raw_buffer.full(), "raw buffer is full!");
            }
        }
    }
}

} //end of anonymous namespace

void stdio::virtual_terminal::print(char key){
    //TODO If it is not the active terminal, buffer it
    k_print(key);
}

void stdio::virtual_terminal::send_input(char key){
    if(!input_thread_pid){
        return;
    }

    // Simply give the input to the input thread
    keyboard_buffer.push(key);
    thor_assert(!keyboard_buffer.full(), "keyboard buffer is full!");

    // Need hint here because it is coming from an IRQ
    scheduler::unblock_process_hint(input_thread_pid);
}

void stdio::virtual_terminal::send_mouse_input(std::keycode key){
    if(!input_thread_pid){
        return;
    }

    // Simply give the input to the input thread
    mouse_buffer.push(size_t(key));
    thor_assert(!mouse_buffer.full(), "mouse buffer is full!");

    // Need hint here because it is coming from an IRQ
    scheduler::unblock_process_hint(input_thread_pid);
}

size_t stdio::virtual_terminal::read_input_can(char* buffer, size_t max){
    size_t read = 0;

    while(true){
        while(!canonical_buffer.empty()){
            auto c = canonical_buffer.pop();

            buffer[read++] = c;

            if(read >= max || c == '\n'){
                return read;
            }
        }

        input_queue.wait();
    }
}

// TODO In case of max < read, the timeout is not respected
size_t stdio::virtual_terminal::read_input_can(char* buffer, size_t max, size_t ms){
    size_t read = 0;

    while(true){
        while(!canonical_buffer.empty()){
            auto c = canonical_buffer.pop();

            buffer[read++] = c;

            if(read >= max || c == '\n'){
                return read;
            }
        }

        if(!ms){
            return read;
        }

        if(!input_queue.wait_for(ms)){
            return read;
        }
    }
}

size_t stdio::virtual_terminal::read_input_raw(){
    if(raw_buffer.empty()){
        input_queue.wait();
    }

    return raw_buffer.pop();
}

size_t stdio::virtual_terminal::read_input_raw(size_t ms){
    if(raw_buffer.empty()){
        if(!ms){
            return static_cast<size_t>(std::keycode::TIMEOUT);
        }

        if(!input_queue.wait_for(ms)){
            return static_cast<size_t>(std::keycode::TIMEOUT);
        }
    }

    thor_assert(!raw_buffer.empty(), "There is a problem with the sleep queue");

    return raw_buffer.pop();
}

void stdio::virtual_terminal::set_canonical(bool can){
    logging::logf(logging::log_level::TRACE, "Switched terminal %u canonical mode from %u to %u\n", id, uint64_t(canonical), uint64_t(can));

    canonical = can;
}

void stdio::virtual_terminal::set_mouse(bool m){
    logging::logf(logging::log_level::TRACE, "Switched terminal %u mouse handling mode from %u to %u\n", id, uint64_t(mouse), uint64_t(m));

    mouse = m;
}

bool stdio::virtual_terminal::is_canonical() const {
    return canonical;
}

void stdio::virtual_terminal::save(){
    buffer = console::save(buffer);
}

void stdio::virtual_terminal::restore(){
    console::restore(buffer);
}

void stdio::init_terminals(){
    size_t id = 0;

    for(auto& terminal : terminals){
        terminal.id        = id++;
        terminal.active    = false;
        terminal.canonical = true;
        terminal.mouse     = false;
    }

    active_terminal = 0;
    terminals[active_terminal].active = true;
}

void stdio::register_devices(){
    for(auto& terminal : terminals){
        std::string name = std::string("tty") + std::to_string(terminal.id);

        devfs::register_device("/dev/", name, devfs::device_type::CHAR_DEVICE, tty_driver, &terminal);
    }
}

void stdio::finalize(){
    for(auto& terminal : terminals){
        auto* user_stack   = new char[scheduler::user_stack_size];
        auto* kernel_stack = new char[scheduler::kernel_stack_size];

        auto& input_process = scheduler::create_kernel_task_args("tty_input", user_stack, kernel_stack, &input_thread, &terminal);
        input_process.ppid     = 1;
        input_process.priority = scheduler::DEFAULT_PRIORITY;

        scheduler::queue_system_process(input_process.pid);

        terminal.input_thread_pid = input_process.pid;

        // Save the initial image of the terminal
        terminal.save();
    }
}

size_t stdio::terminals_count(){
    return MAX_TERMINALS;
}

stdio::virtual_terminal& stdio::get_active_terminal(){
    return terminals[active_terminal];
}

stdio::virtual_terminal& stdio::get_terminal(size_t id){
    thor_assert(id < MAX_TERMINALS, "Out of bound tty");

    return terminals[id];
}

void stdio::switch_terminal(size_t id){
    if(active_terminal != id){
        logging::logf(logging::log_level::TRACE, "stdio: Switch activate virtual terminal %u\n", id);

        terminals[active_terminal].save();
        terminals[id].restore();

        active_terminal = id;
    }
}

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
