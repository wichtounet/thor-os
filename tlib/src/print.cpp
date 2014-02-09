//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <stdarg.h>

#include "print.hpp"

void print(char c){
    asm volatile("mov rax, 0; mov rbx, %0; int 50"
        : //No outputs
        : "r" (static_cast<size_t>(c))
        : "rax", "rbx");
}

void print(const char* s){
    asm volatile("mov rax, 1; mov rbx, %0; int 50"
        : //No outputs
        : "r" (reinterpret_cast<size_t>(s))
        : "rax", "rbx");
}

void print(size_t v){
    asm volatile("mov rax, 2; mov rbx, %0; int 50"
        : //No outputs
        : "r" (v)
        : "rax", "rbx");
}

void print(const std::string& s){
    return print(s.c_str());
}

void print_line(){
    print('\n');
}

void print_line(const char* s){
    print(s);
    print_line();
}

void print_line(size_t v){
    print(v);
    print_line();
}

size_t read_input(char* buffer, size_t max){
    size_t value;
    asm volatile("mov rax, 3; int 50; mov %0, rax"
        : "=m" (value)
        : "b" (buffer), "c" (max)
        : "rax");
    return value;
}

void  clear(){
    asm volatile("mov rax, 100; int 50;"
        : //No outputs
        : //No inputs
        : "rax");
}

size_t get_columns(){
    size_t value;
    asm volatile("mov rax, 101; int 50; mov %0, rax"
        : "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

size_t get_rows(){
    size_t value;
    asm volatile("mov rax, 102; int 50; mov %0, rax"
        : "=m" (value)
        : //No inputs
        : "rax");
    return value;
}

std::string vsprintf(const std::string& format, va_list va){
    std::string s(format.size());

    char ch;
    int fi = 0;

    while ((ch = format[fi++])) {
        if(ch != '%'){
            s += ch;
        } else {
            ch = format[fi++];

            size_t min_width = 0;
            while(ch >= '0' && ch <= '9'){
                min_width = 10 * min_width + (ch - '0');
                ch = format[fi++];
            }

            size_t min_digits = 0;
            if(ch == '.'){
                ch = format[fi++];

                while(ch >= '0' && ch <= '9'){
                    min_digits = 10 * min_digits + (ch - '0');
                    ch = format[fi++];
                }
            }

            auto prev = s.size();

            //Signed decimal
            if(ch == 'd'){
                auto arg = va_arg(va, int64_t);

                if(min_digits > 0){
                    size_t d = std::digits(arg);
                    if(min_digits > d){
                        min_digits -= d;

                        if(arg < 0){
                            arg *= -1;
                            s += '-';
                        }

                        while(min_digits > 0){
                            while(min_digits > 0){
                                arg += '0';
                                --min_digits;
                            }
                        }
                    }
                }

                s += std::to_string(arg);
            }
            //Unsigned Decimal
            else if(ch == 'u'){
                auto arg = va_arg(va, uint64_t);

                if(min_digits > 0){
                    size_t d = std::digits(arg);
                    if(min_digits > d){
                        min_digits -= d;
                        while(min_digits > 0){
                            while(min_digits > 0){
                                arg += '0';
                                --min_digits;
                            }
                        }
                    }
                }

                s += std::to_string(arg);
            }
            //Hexadecimal
            else if(ch == 'h'){
                s += "0x";

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
                        arg += '0';
                        --min_digits;
                    }
                }

                while(true){
                    uint8_t digit = buffer[i];

                    if(digit < 10){
                        s += static_cast<char>('0' + digit);
                    } else {
                        switch(digit){
                        case 10:
                            s += 'A';
                            break;
                        case 11:
                            s += 'B';
                            break;
                        case 12:
                            s += 'C';
                            break;
                        case 13:
                            s += 'D';
                            break;
                        case 14:
                            s += 'E';
                            break;
                        case 15:
                            s += 'F';
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
                    s += std::to_string(memory / (1024 * 1024 * 1024));
                    s += "GiB";
                } else if(memory >= 1024 * 1024){
                    s += std::to_string(memory / (1024 * 1024));
                    s += "MiB";
                } else if(memory >= 1024){
                    s += std::to_string(memory / 1024);
                    s += "KiB";
                } else {
                    s += std::to_string(memory);
                    s += "B";
                }
            }
            //String
            else if(ch == 's'){
                auto arg = va_arg(va, const char*);
                s += arg;
            }

            if(min_width > 0){
                size_t width = s.size() - prev;

                if(min_width > width){
                    min_width -= width;

                    while(min_width > 0){
                        s += ' ';
                        --min_width;
                    }
                }
            }
        }
    }

    return std::move(s);
}

std::string sprintf(const std::string& format, ...){
    va_list va;
    va_start(va, format);

    auto s = vsprintf(format, va);

    va_end(va);

    return std::move(s);
}

void printf(const std::string& format, ...){
    va_list va;
    va_start(va, format);

    auto s = vsprintf(format, va);
    print(s);

    va_end(va);
}
