//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <stdarg.h>

#include <types.hpp>
#include <string.hpp>

#include "print.hpp"
#include "stdio.hpp"

namespace {

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

} // end of anonymous namespace

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

void k_print(char key){
    stdio::get_active_terminal().print(key);
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
    for(uint64_t i = 0; i < end && str[i] != 0; ++i){
        k_print(str[i]);
    }
}

#include "printf_def.hpp"

void __printf(const std::string& str){
    k_print(str);
}

void __printf_raw(const char* str){
    k_print(str);
}
