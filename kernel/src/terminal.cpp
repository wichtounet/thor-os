//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "stl/vector.hpp"

#include "terminal.hpp"
#include "keyboard.hpp"
#include "console.hpp"

namespace {

constexpr const size_t DEFAULT_TERMINALS = 1;
size_t active_terminal;

bool shift = false;

std::vector<stdio::virtual_terminal> terminals;

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
            }
        }
        //Key pressed
        else {
            if(key == keyboard::KEY_LEFT_SHIFT || key == keyboard::KEY_RIGHT_SHIFT){
                shift = true;
            } else if(key == keyboard::KEY_BACKSPACE){
                if(!input_buffer.empty()){
                    print('\b');
                    input_buffer.pop();
                }
            } else {
                auto qwertz_key =
                    shift
                    ? keyboard::shift_key_to_ascii(key)
                    : keyboard::key_to_ascii(key);

                if(qwertz_key){
                    if(input_queue.empty()){
                        input_buffer.push(qwertz_key);
                    } else {
                        //TODO
                    }

                    print(qwertz_key);
                }
            }
        }
    } else {
        //TODO
    }
}

size_t stdio::virtual_terminal::read_input(char* buffer, size_t max){
    size_t read = 0;

    while(read < max && !input_buffer.empty()){
        buffer[read] = input_buffer.pop();

        if(buffer[read] == '\n'){
            ++read;
            break;
        }

        ++read;
    }

    if(read == max || buffer[read] == '\n'){
        scheduler::get_process(scheduler::get_pid()).context->rax = read;
    } else {
        input_queue.sleep();
    }

    return 0;
}

void stdio::init_terminals(){
    terminals.resize(DEFAULT_TERMINALS);

    for(size_t i  = 0; i < DEFAULT_TERMINALS; ++i){
        terminals[i].id = i;
        terminals[i].active = false;
        terminals[i].canonical = true;
    }

    active_terminal = 0;
    terminals[active_terminal].active = true;
}

stdio::virtual_terminal& stdio::get_active_terminal(){
    return terminals[active_terminal];
}

stdio::virtual_terminal& stdio::get_terminal(size_t id){
    return terminals[id];
}
