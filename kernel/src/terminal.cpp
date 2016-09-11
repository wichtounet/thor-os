//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
bool ctrl = false;

std::array<stdio::virtual_terminal, MAX_TERMINALS> terminals;

} //end of anonymous namespace

void stdio::virtual_terminal::print(char key){
    //TODO If it is not the active terminal, buffer it
    k_print(key);
}

void stdio::virtual_terminal::send_input(char key){
    if(canonical){
        //Key released
        if(key & 0x80){
            key &= ~(0x80);
            if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = false;
            } else if(key == keyboard::KEY_LEFT_CTRL){
                ctrl = false;
            }
        }
        //Key pressed
        else {
            if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = true;
            } else if(key == keyboard::KEY_LEFT_CTRL){
                ctrl = true;
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
                    if(ctrl && qwertz_key == 'c'){
                        input_buffer.push(200);
                    } else {
                        input_buffer.push(qwertz_key);

                        print(qwertz_key);
                    }

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

        if(read > 0 && (c == '\n' || c == 200 || read == max)){
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
