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

template<typename Iterator1, typename Iterator2>
size_t compare_n(Iterator1 it1, Iterator2 it2, size_t n){
    while(n--){
        if(*it1 != *it2){
            return *it1- *it2;
        } else {
            ++it1;
            ++it2;
        }
    }

    return 0;
}

template<typename Iterator1, typename Iterator2>
bool equal_n(Iterator1 it1, Iterator2 it2, size_t n){
    return compare_n(it1, it2, n) == 0;
}

} //end of namespace std

template<typename CharT>
struct basic_string;
typedef basic_string<char> string;

uint64_t parse(const char* str);
uint64_t parse(const char* str, const char* end);
uint64_t parse(const string& str);

uint64_t str_len(const char* a);

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
