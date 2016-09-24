//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <stdarg.h>

#include <types.hpp>
#include <string.hpp>

#include "assert.hpp"
#include "console.hpp"
#include "vesa.hpp"

#include "text_console.hpp"
#include "vesa_console.hpp"

namespace {

text_console t_console;
vesa_console v_console;
bool text = true;

void clear(){
    if(text){
        t_console.clear();
    } else {
        v_console.clear();
    }
}

void scroll_up(){
    if(text){
        t_console.scroll_up();
    } else {
        v_console.scroll_up();
    }
}

void print_char(size_t line, size_t column, char c){
    if(text){
        t_console.print_char(line, column, c);
    } else {
        v_console.print_char(line, column, c);
    }
}

volatile size_t current_line = 0;
volatile size_t current_column = 0;

void next_line(){
    ++current_line;

    if(current_line == console::get_rows()){
        scroll_up();

        --current_line;
    }

    current_column = 0;
}

} //end of anonymous namespace

void console::init(){
    text = !vesa::enabled();

    if(text){
        t_console.init();
    } else {
        v_console.init();
    }
}

size_t console::get_rows(){
    if(text){
        return t_console.lines();
    } else {
        return v_console.lines();
    }
}

size_t console::get_columns(){
    if(text){
        return t_console.columns();
    } else {
        return v_console.columns();
    }
}

struct console_state {
    size_t current_line;
    size_t current_column;
    void* buffer = nullptr;
};

void* console::save(void* buffer){
    thor_assert(!text, "save/restore of the text console is not yet supported");

    auto* state = static_cast<console_state*>(buffer);
    if(!state){
        state = new console_state;
    }

    state->current_line   = current_line;
    state->current_column = current_column;

    state->buffer = v_console.save(state->buffer);

    return state;
}

void console::restore(void* buffer){
    thor_assert(!text, "save/restore of the text console is not yet supported");

    auto* state = static_cast<console_state*>(buffer);

    current_line   = state->current_line;
    current_column = state->current_column;

    v_console.restore(state->buffer);
}

void console::set_column(size_t column){
    current_column = column;
}

size_t console::get_column(){
    return current_column;
}

void console::set_line(size_t line){
    current_line = line;
}

size_t console::get_line(){
    return current_line;
}

void console::wipeout(){
    clear();

    current_line = 0;
    current_column = 0;
}

//void k_print(char key){
    //if(key == '\n'){
        //next_line();
    //} else if(key == '\r'){
        //// Ignore \r for now
    //} else if(key == '\b'){
        //--current_column;
        //k_print(' ');
        //--current_column;
    //} else if(key == '\t'){
        //k_print("  ");
    //} else {
        //print_char(current_line, current_column, key);

        //++current_column;

        //if(current_column == console::get_columns()){
            //next_line();
        //}
    //}
//}
