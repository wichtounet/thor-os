//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <stdarg.h>

#include "stl/types.hpp"
#include "stl/string.hpp"

#include "console.hpp"
#include "vesa.hpp"

#include "text_console.hpp"
#include "vesa_console.hpp"

namespace {

text_console t_console;
vesa_console v_console;
bool text = true;

size_t lines(){
    if(text){
        return t_console.lines();
    } else {
        return v_console.lines();
    }
}

size_t columns(){
    if(text){
        return t_console.columns();
    } else {
        return v_console.columns();
    }
}

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

long current_line = 0;
long current_column = 0;

template<typename N>
uint64_t digits(N number){
    if(number < 10){
        return 1;
    }

    uint64_t i = 0;

    while(number != 0){
        number /= 10;
        ++i;
    }

    return i;
}

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

void set_column(long column){
    current_column = column;
}

long get_column(){
    return current_column;
}

void set_line(long line){
    current_line= line;
}

long get_line(){
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

    if(current_line == lines()){
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

        if(current_column == columns()){
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

void k_printf(const char* fmt, ...){
    va_list va;
    va_start(va, fmt);

    char ch;

    while ((ch=*(fmt++))) {
        if(ch != '%'){
            k_print(ch);
        } else {
            ch = *(fmt++);

            size_t min_width = 0;
            while(ch >= '0' && ch <= '9'){
                min_width = 10 * min_width + (ch - '0');
                ch = *(fmt++);
            }

            size_t min_digits = 0;
            if(ch == '.'){
                ch = *(fmt++);

                while(ch >= '0' && ch <= '9'){
                    min_digits = 10 * min_digits + (ch - '0');
                    ch = *(fmt++);
                }
            }

            auto prev = current_column;

            //Signed decimal
            if(ch == 'd'){
                auto arg = va_arg(va, int64_t);

                if(min_digits > 0){
                    size_t d = digits(arg);
                    if(min_digits > d){
                        min_digits -= d;

                        if(arg < 0){
                            arg *= -1;
                            k_print('-');
                        }

                        while(min_digits > 0){
                            while(min_digits > 0){
                                k_print('0');
                                --min_digits;
                            }
                        }
                    }
                }

                k_print(arg);
            }
            //Unsigned Decimal
            else if(ch == 'u'){
                auto arg = va_arg(va, uint64_t);

                if(min_digits > 0){
                    size_t d = digits(arg);
                    if(min_digits > d){
                        min_digits -= d;
                        while(min_digits > 0){
                            while(min_digits > 0){
                                k_print('0');
                                --min_digits;
                            }
                        }
                    }
                }

                k_print(arg);
            }
            //Hexadecimal
            else if(ch == 'h'){
                k_print("0x");

                uint8_t buffer[20];

                auto arg = va_arg(va, uint64_t);
                size_t i = 0;

                while(arg / 16 != 0){
                    buffer[i++] = arg % 16;
                    arg /= 16;
                }

                buffer[i] = arg;

                if(min_digits > 0 && min_digits > i){
                    min_digits -= i + 1;
                    while(min_digits > 0){
                        k_print('0');
                        --min_digits;
                    }
                }

                while(true){
                    uint8_t digit = buffer[i];

                    if(digit < 10){
                        k_print(static_cast<char>('0' + digit));
                    } else {
                        switch(digit){
                        case 10:
                            k_print('A');
                            break;
                        case 11:
                            k_print('B');
                            break;
                        case 12:
                            k_print('C');
                            break;
                        case 13:
                            k_print('D');
                            break;
                        case 14:
                            k_print('E');
                            break;
                        case 15:
                            k_print('F');
                            break;
                        }
                    }

                    if(i == 0){
                        break;
                    }

                    --i;
                }
            }
            //Memory
            else if(ch == 'm'){
                auto memory= va_arg(va, uint64_t);

                if(memory >= 1024 * 1024 * 1024){
                    k_print(memory / (1024 * 1024 * 1024));
                    k_print("GiB");
                } else if(memory >= 1024 * 1024){
                    k_print(memory / (1024 * 1024));
                    k_print("MiB");
                } else if(memory >= 1024){
                    k_print(memory / 1024);
                    k_print("KiB");
                } else {
                    k_print(memory);
                    k_print("B");
                }
            }
            //String
            else if(ch == 's'){
                auto arg = va_arg(va, const char*);
                k_print(arg);
            }

            if(min_width > 0){
                size_t width = current_column - prev;

                if(min_width > width){
                    min_width -= width;

                    while(min_width > 0){
                        k_print(' ');
                        --min_width;
                    }
                }
            }
        }
    }

    va_end(va);
}
