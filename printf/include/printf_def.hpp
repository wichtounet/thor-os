//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

inline size_t str_cat(char* destination, const char* source){
    size_t n = 0;

    while(*source){
        *destination++ = *source++;
        ++n;
    }

    return n;
}

inline size_t str_cat(char* destination, const char* source, size_t length){
    size_t n = 0;

    while(n < length){
        *destination++ = *source++;
        ++n;
    }

    return n;
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
            bool variable_length = false;

            if (ch == '.') {
                ch = format[fi++];

                if (ch == '*') {
                    variable_length = true;
                    ch = format[fi++];
                } else {
                    while (ch >= '0' && ch <= '9') {
                        min_digits = 10 * min_digits + (ch - '0');
                        ch         = format[fi++];
                    }
                }
            }

            auto prev = s.size();

            //Signed decimal
            if(ch == 'd'){
                int64_t arg = va_arg(va, int64_t);

                if(min_digits > 0){
                    size_t d = std::digits(arg);
                    if(min_digits > d){
                        min_digits -= d;

                        if(arg < 0){
                            arg *= -1;
                            s += '-';
                        }

                        while(min_digits > 0){
                            s += '0';
                            --min_digits;
                        }
                    }
                }

                s += std::to_string(arg);
            }
            //Unsigned Decimal
            else if(ch == 'u'){
                uint64_t arg = va_arg(va, uint64_t);

                if(min_digits > 0){
                    size_t d = std::digits(arg);
                    if(min_digits > d){
                        min_digits -= d;
                        while(min_digits > 0){
                            s += '0';
                            --min_digits;
                        }
                    }
                }

                s += std::to_string(arg);
            }
            //Hexadecimal
            else if(ch == 'h' || ch == 'x' || ch == 'p'){
                if(ch == 'h' || ch == 'p'){
                    s += "0x";
                }

                uint8_t buffer[20];

                uint64_t arg = va_arg(va, uint64_t);
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
                uint64_t memory= va_arg(va, uint64_t);

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
            // Boolean
            else if(ch == 'b'){
                bool value= va_arg(va, int);

                if(value){
                    s += "true";
                } else {
                    s += "false";
                }
            }
            // Binary
            else if(ch == 'B'){
                size_t value= va_arg(va, size_t);

                uint8_t bits[64];
                size_t top = 0;
                bits[0] = 0;

                while(value){
                    bits[top++] = value & 1;
                    value >>= 1;
                }

                s += "0b";

                for(size_t i = top; i > 0; --i){
                    s += bits[i] ? '1' : '0';
                }

                s += bits[0] ? '1' : '0';
            }
            //String
            else if(ch == 's'){
                if (variable_length) {
                    size_t size_arg = va_arg(va, size_t);

                    const char* arg = va_arg(va, const char*);

                    if (arg) {
                        s.append(arg, arg + size_arg);
                    }
                } else {
                    const char* arg = va_arg(va, const char*);

                    if (arg) {
                        s += arg;
                    }
                }
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

    return s;
}

std::string sprintf(const std::string& format, ...){
    va_list va;
    va_start(va, format);

    auto s = vsprintf(format, va);

    va_end(va);

    return s;
}

void vprintf(const std::string& format, va_list va){
    __printf(vsprintf(format, va));
}

void printf(const std::string& format, ...){
    va_list va;
    va_start(va, format);

    vprintf(format, va);

    va_end(va);
}

// Raw versions

//TODO Check for n
void vsprintf_raw(char* out_buffer, size_t /*n*/, const char* format, va_list va){
    char ch;
    int fi = 0;

    size_t out_i = 0;

    while ((ch = format[fi++])) {
        if(ch != '%'){
            out_buffer[out_i++] = ch;
        } else {
            ch = format[fi++];

            size_t min_width = 0;
            while(ch >= '0' && ch <= '9'){
                min_width = 10 * min_width + (ch - '0');
                ch = format[fi++];
            }

            size_t min_digits = 0;
            bool variable_length = false;

            if (ch == '.') {
                ch = format[fi++];

                if (ch == '*') {
                    variable_length = true;
                    ch = format[fi++];
                } else {
                    while (ch >= '0' && ch <= '9') {
                        min_digits = 10 * min_digits + (ch - '0');
                        ch         = format[fi++];
                    }
                }
            }

            auto prev = out_i;

            //Signed decimal
            if(ch == 'd'){
                int64_t arg = va_arg(va, int64_t);

                if(min_digits > 0){
                    size_t d = std::digits(arg);
                    if(min_digits > d){
                        min_digits -= d;

                        if(arg < 0){
                            arg *= -1;
                            out_buffer[out_i++] = '-';
                        }

                        while(min_digits > 0){
                            out_buffer[out_i++] = '0';
                            --min_digits;
                        }
                    }
                }

                char buffer[32];
                std::to_raw_string(arg, buffer, 32);
                out_i += str_cat(out_buffer + out_i, buffer);
            }
            //Unsigned Decimal
            else if(ch == 'u'){
                uint64_t arg = va_arg(va, uint64_t);

                if(min_digits > 0){
                    size_t d = std::digits(arg);
                    if(min_digits > d){
                        min_digits -= d;
                        while(min_digits > 0){
                            out_buffer[out_i++] = '0';
                            --min_digits;
                        }
                    }
                }

                char buffer[32];
                std::to_raw_string(arg, buffer, 32);
                out_i += str_cat(out_buffer + out_i, buffer);
            }
            //Hexadecimal
            else if(ch == 'h' || ch == 'x' || ch == 'p'){
                if(ch == 'h' || ch == 'p'){
                    out_buffer[out_i++] = '0';
                    out_buffer[out_i++] = 'x';
                }

                uint8_t buffer[20];

                uint64_t arg = va_arg(va, uint64_t);
                size_t i = 0;

                while(arg / 16 != 0){
                    buffer[i++] = arg % 16;
                    arg /= 16;
                }

                buffer[i] = arg;

                if(min_digits > 0 && min_digits > i){
                    min_digits -= i + 1;
                    while(min_digits > 0){
                        out_buffer[out_i++] = '0';
                        --min_digits;
                    }
                }

                while(true){
                    uint8_t digit = buffer[i];

                    if(digit < 10){
                        out_buffer[out_i++] = static_cast<char>('0' + digit);
                    } else {
                        switch(digit){
                        case 10:
                            out_buffer[out_i++] = 'A';
                            break;
                        case 11:
                            out_buffer[out_i++] = 'B';
                            break;
                        case 12:
                            out_buffer[out_i++] = 'C';
                            break;
                        case 13:
                            out_buffer[out_i++] = 'D';
                            break;
                        case 14:
                            out_buffer[out_i++] = 'E';
                            break;
                        case 15:
                            out_buffer[out_i++] = 'F';
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
                uint64_t memory= va_arg(va, uint64_t);

                char buffer[32];

                if(memory >= 1024 * 1024 * 1024){
                    std::to_raw_string(memory / (1024 * 1024 * 1024), buffer, 32);
                    out_i += str_cat(out_buffer + out_i, buffer);
                    out_i += str_cat(out_buffer + out_i, "GiB");
                } else if(memory >= 1024 * 1024){
                    std::to_raw_string(memory / (1024 * 1024), buffer, 32);
                    out_i += str_cat(out_buffer + out_i, buffer);
                    out_i += str_cat(out_buffer + out_i, "MiB");
                } else if(memory >= 1024){
                    std::to_raw_string(memory / 1024, buffer, 32);
                    out_i += str_cat(out_buffer + out_i, buffer);
                    out_i += str_cat(out_buffer + out_i, "KiB");
                } else {
                    std::to_raw_string(memory, buffer, 32);
                    out_i += str_cat(out_buffer + out_i, buffer);
                    out_i += str_cat(out_buffer + out_i, "B");
                }
            }
            // Boolean
            else if(ch == 'b'){
                bool value= va_arg(va, int);

                if(value){
                    out_i += str_cat(out_buffer + out_i, "true");
                } else {
                    out_i += str_cat(out_buffer + out_i, "false");
                }
            }
            // Binary
            else if(ch == 'B'){
                size_t value= va_arg(va, size_t);

                uint8_t bits[64];
                size_t top = 0;
                bits[0] = 0;

                while(value){
                    bits[top++] = value & 1;
                    value >>= 1;
                }

                out_buffer[out_i++] = '0';
                out_buffer[out_i++] = 'b';

                for(size_t i = top; i > 0; --i){
                    out_buffer[out_i++] = bits[i] ? '1' : '0';
                }

                out_buffer[out_i++] = bits[0] ? '1' : '0';
            }
            //String
            else if(ch == 's'){
                if (variable_length) {
                    size_t size_arg = va_arg(va, size_t);

                    const char* arg = va_arg(va, const char*);

                    if (arg) {
                        out_i += str_cat(out_buffer + out_i, arg, size_arg);
                    }
                } else {
                    const char* arg = va_arg(va, const char*);

                    if (arg) {
                        out_i += str_cat(out_buffer + out_i, arg);
                    }
                }
            }

            if(min_width > 0){
                size_t width = out_i - prev;

                if(min_width > width){
                    min_width -= width;

                    while(min_width > 0){
                        out_buffer[out_i++] = ' ';
                        --min_width;
                    }
                }
            }
        }
    }

    out_buffer[out_i++] = '\0';
}

void sprintf_raw(char* buffer, size_t n, const char* format, ...){
    va_list va;
    va_start(va, format);

    vsprintf_raw(buffer, n, format, va);

    va_end(va);
}

void vprintf_raw(const char* format, va_list va){
    char buffer[1024];
    vsprintf_raw(buffer, 1024, format, va);
    __printf_raw(buffer);
}

void printf_raw(const char* format, ...){
    va_list va;
    va_start(va, format);

    vprintf_raw(format, va);

    va_end(va);
}
