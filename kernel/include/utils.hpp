//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILS_H
#define UTILS_H

#include "types.hpp"

namespace std {

template<typename ForwardIterator, typename T>
void fill(ForwardIterator it, ForwardIterator end, const T& value){
    while(it != end){
        *it = value;
        ++it;
    }
}

template<typename ForwardIterator, typename T>
void fill_n(ForwardIterator it, size_t n, const T& value){
    while(n--){
        *it = value;
        ++it;
    }
}

} //end of namespace std

template<typename CharT>
struct basic_string;
typedef basic_string<char> string;

uint64_t parse(const char* str);
uint64_t parse(const char* str, const char* end);
uint64_t parse(const string& str);

uint64_t str_len(const char* a);

int memcmp(const void* s1, const void* s2, size_t n);
void memcopy(void* destination, const void* source, size_t n);

template< class T > struct remove_reference      {typedef T type;};
template< class T > struct remove_reference<T&>  {typedef T type;};
template< class T > struct remove_reference<T&&> {typedef T type;};

template<typename T>
constexpr typename remove_reference<T>::type&& move(T&& t){
    return static_cast<typename remove_reference<T>::type&&>(t);
}

template<typename T>
constexpr T&& forward(typename remove_reference<T>::type& t ){
    return static_cast<T&&>(t);
}

#endif
