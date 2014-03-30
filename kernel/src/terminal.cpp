//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <array.hpp>

#include "terminal.hpp"
#include "keyboard.hpp"
#include "console.hpp"
#include "assert.hpp"

namespace {

constexpr const size_t MAX_TERMINALS = 2;
size_t active_terminal;

bool shift = false;

std::array<stdio::virtual_terminal, MAX_TERMINALS> terminals;

void tasklet_handle_input(size_t d1, size_t d2){
    auto key = static_cast<char>(d1);
    auto term = reinterpret_cast<stdio::virtual_terminal*>(d2);

    term->handle_input(key);
}

} //end of anonymous namespace

void stdio::virtual_terminal::print(char key){
    //TODO If it is not the active terminal, buffer it
    k_print(key);
}

void stdio::virtual_terminal::send_input(char key){
    scheduler::tasklet task;
    task.fun = &tasklet_handle_input;
    task.d1 = key;
    task.d2 = reinterpret_cast<size_t>(this);

    scheduler::irq_register_tasklet(task, scheduler::DEFAULT_PRIORITY);
}

void stdio::virtual_terminal::handle_input(char key){
    if(canonical){
        std::lock_guard<spinlock> l(terminal_lock);

        //Key released
        if(key & 0x80){
            key &= ~(0x80);
            if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = false;
            }
        }
        //Key pressed
        else {
            if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = true;
            } else if(key == keyboard::KEY_BACKSPACE){
                if(!input_buffer.empty() || !canonical_buffer.empty()){
                    print('\b');
                }

                input_buffer.push('\b');
            } else {
                auto qwertz_key =
                    shift
                    ? keyboard::shift_key_to_ascii(key)
                    : keyboard::key_to_ascii(key);

                if(qwertz_key){
                    input_buffer.push(qwertz_key);

                    print(qwertz_key);

                    if(!input_queue.empty()){
                        input_queue.wake_up();
                    }
                }
            }
        }
    } else {
        //TODO
    }
}

size_t stdio::virtual_terminal::read_input(char* buffer, size_t max){
    size_t read = 0;
    char c;

    while(true){
        {
            std::lock_guard<spinlock> l(terminal_lock);

            while(read < max && !input_buffer.empty()){
                c = input_buffer.pop();

                canonical_buffer.push(c);

                if(c == '\b'){
                    if(read > 0){
                        --read;
                    }
                } else {
                    ++read;

                    if(c == '\n'){
                        break;
                    }
                }
            }

            if(read > 0 && (c == '\n' || read == max)){
                read = 0;
                while(!canonical_buffer.empty()){
                    auto value = canonical_buffer.pop();

                    if(value == '\b'){
                        --read;
                    } else {
                        buffer[read++] = value;
                    }
                }

                return read;
            }
        }

        input_queue.sleep();
    }
}

void stdio::init_terminals(){
    for(size_t i  = 0; i < MAX_TERMINALS; ++i){
        auto& terminal = terminals[i];

        terminal.id = i;
        terminal.active = false;
        terminal.canonical = true;
    }

    active_terminal = 0;
    terminals[active_terminal].active = true;
}

stdio::virtual_terminal& stdio::get_active_terminal(){
    return terminals[active_terminal];
}

stdio::virtual_terminal& stdio::get_terminal(size_t id){
    thor_assert(id < MAX_TERMINALS, "Out of bound tty");

    return terminals[id];
}
