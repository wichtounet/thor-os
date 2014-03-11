//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

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

void printf(const std::string& format, va_list va){
    __printf(vsprintf(format, va));
}

void printf(const std::string& format, ...){
    va_list va;
    va_start(va, format);

    printf(format, va);

    va_end(va);
}
