
#include "utils.hpp"

std::size_t parse(const char* str){
    int i = 0;

    const char* it = str;
    while(*++it){
        ++i;
    }

    std::size_t factor = 1;
    std::size_t acc = 0;

    for(; i >= 0; --i){
        acc += (str[i] - '0') * factor;
        factor *= 10;
    }

    return acc;
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
