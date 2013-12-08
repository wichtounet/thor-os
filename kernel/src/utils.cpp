//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "utils.hpp"
#include "string.hpp"

void memset(void* ptr, unsigned char value, size_t num){
    auto p = static_cast<unsigned char*>(ptr);

    --p;

    while(num--){
        *++p = value;
    }
}

bool str_equals(const char* a, const char* b){
    while(*a && *a == *b){
        ++a;
        ++b;
    }

    return *a == *b;
}

uint64_t parse(const char* it, const char* end){
    int i = end - it - 1;

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (it[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

uint64_t parse(const char* str){
    int i = 0;

    const char* it = str;
    while(*++it){
        ++i;
    }

    uint64_t factor = 1;
    uint64_t acc = 0;

    for(; i >= 0; --i){
        acc += (str[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
}

uint64_t parse(const string& str){
    return parse(str.begin(), str.end());
}

bool str_contains(const char* a, char c){
    while(*a){
        if(*a == c){
            return true;
        }
        ++a;
    }

    return false;
}

void str_copy(const char* a, char* b){
    while(*a){
        *b++ = *a++;
    }

    *b = '\0';
}

uint64_t str_len(const char* a){
    uint64_t length = 0;
    while(*a++){
        ++length;
    }
    return length;
}

const char* str_until(char* a, char c){
    char* it = a;
    while(*it){
        if(*it == c){
            *it = '\0';
            return a;
        }
        ++it;
    }

    return a;
}

const char* str_from(char* a, char c){
    char* it = a;
    while(*it){
        if(*it == c){
            return ++it;
        }
        ++it;
    }

    return a;
}
