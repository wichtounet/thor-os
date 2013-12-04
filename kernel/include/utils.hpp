//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include "types.hpp"

template<typename CharT>
struct basic_string;
typedef basic_string<char> string;

uint64_t parse(const char* str);
uint64_t parse(const char* str, const char* end);
uint64_t parse(const string& str);

bool str_equals(const char* a, const char* b);
bool str_contains(const char* a, char c);
void str_copy(const char* a, char* b);
uint64_t str_len(const char* a);
const char* str_until(char* a, char c);
const char* str_from(char* a, char c);

template<typename T>
void memcopy(T* destination, const T* source, uint64_t size){
    --source;
    --destination;

    while(size--){
        *++destination = *++source;
    }
}

template< class T > struct remove_reference      {typedef T type;};
template< class T > struct remove_reference<T&>  {typedef T type;};
template< class T > struct remove_reference<T&&> {typedef T type;};

template< class T >
constexpr typename remove_reference<T>::type&& move( T&& t ){
    return static_cast<typename remove_reference<T>::type&&>(t);
}

#endif
