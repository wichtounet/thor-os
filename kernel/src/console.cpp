//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <stdarg.h>

#include <types.hpp>
#include <string.hpp>

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

template<int B, typename D>
void print_unsigned(D number){
    if(number == 0){
        k_print('0');
        return;
    }

    char buffer[B];
    int i = 0;

    while(number != 0){
        buffer[i++] = '0' + number % 10;
        number /= 10;
    }

    --i;

    for(; i >= 0; --i){
        k_print(buffer[i]);
    }
}

template<int B, typename U, typename D>
void print_signed(D number){
    if(number < 0){
        k_print('-');
        print_unsigned<B>(static_cast<U>(-1 * number));
    } else {
        print_unsigned<B>(static_cast<U>(number));
    }
}

} //end of anonymous namespace

void init_console(){
    text = !vesa::vesa_enabled;

    if(text){
        t_console.init();
    } else {
        v_console.init();
    }
}

size_t get_rows(){
    if(text){
        return t_console.lines();
    } else {
        return v_console.lines();
    }
}

size_t get_columns(){
    if(text){
        return t_console.columns();
    } else {
        return v_console.columns();
    }
}

void set_column(size_t column){
    current_column = column;
}

size_t get_column(){
    return current_column;
}

void set_line(size_t line){
    current_line = line;
}

size_t get_line(){
    return current_line;
}

void k_print(uint8_t number){
    print_unsigned<3>(number);
}

void k_print(uint16_t number){
    print_unsigned<5>(number);
}

void k_print(uint32_t number){
    print_unsigned<10>(number);
}

void k_print(uint64_t number){
    print_unsigned<20>(number);
}

void k_print(int8_t number){
    print_signed<3,uint8_t>(number);
}

void k_print(int16_t number){
    print_signed<5,uint16_t>(number);
}

void k_print(int32_t number){
    print_signed<10,uint32_t>(number);
}

void k_print(int64_t number){
    print_signed<20,uint64_t,int64_t>(number);
}

void next_line(){
    ++current_line;

    if(current_line == get_rows()){
        scroll_up();

        --current_line;
    }

    current_column = 0;
}

void k_print(char key){
    if(key == '\n'){
        next_line();
    } else if(key == '\b'){
        --current_column;
        k_print(' ');
        --current_column;
    } else if(key == '\t'){
        k_print("  ");
    } else {
        print_char(current_line, current_column, key);

        ++current_column;

        if(current_column == get_columns()){
            next_line();
        }
    }
}

void k_print(const char* str){
    for(uint64_t i = 0; str[i] != 0; ++i){
        k_print(str[i]);
    }
}

void k_print(const std::string& s){
    for(auto c : s){
        k_print(c);
    }
}

void k_print(const char* str, uint64_t end){
    for(uint64_t i = 0; str[i] != 0 && i < end; ++i){
        k_print(str[i]);
    }
}

void wipeout(){
    clear();

    current_line = 0;
    current_column = 0;
}

#include "printf_def.hpp"

void __printf(const std::string& str){
    k_print(str);
}
