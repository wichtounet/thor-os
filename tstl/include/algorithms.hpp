//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <type_traits.hpp>

#include <types.hpp>

namespace std {

template<typename T>
constexpr typename remove_reference<T>::type&& move(T&& t){
    return static_cast<typename remove_reference<T>::type&&>(t);
}

template<typename T>
constexpr T&& forward(typename remove_reference<T>::type& t ){
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& forward(typename remove_reference<T>::type&& t ){
    return static_cast<T&&>(t);
}

template<typename InputIterator, typename OutputIterator>
void copy(OutputIterator out, InputIterator it, InputIterator end){
    if(it != end){
        *out = *it;

        while(++it != end){
            *++out = *it;
        }
    }
}

template<typename InputIterator, typename OutputIterator>
void copy_n(OutputIterator out, InputIterator in, size_t n){
    if(n > 0){
        *out = *in;

        while(--n){
            *++out = *++in;
        }
    }
}

template<typename InputIterator, typename OutputIterator>
void move_n(OutputIterator out, InputIterator in, size_t n){
    if(n > 0){
        *out = std::move(*in);

        while(--n){
            *++out = std::move(*++in);
        }
    }
}

template<typename ForwardIterator, typename T>
void fill(ForwardIterator it, ForwardIterator end, const T& value){
    if(it != end){
        *it = value;

        while(++it != end){
            *it = value;
        }
    }
}

template<typename ForwardIterator, typename T>
void fill_n(ForwardIterator it, size_t n, const T& value){
    if(n > 0){
        *it = value;

        while(--n){
            *++it = value;
        }
    }
}

template<typename Iterator1, typename Iterator2>
size_t compare_n(Iterator1 it1, Iterator2 it2, size_t n){
    if(n > 0){
        while(n--){
            if(*it1 != *it2){
                return *it1- *it2;
            } else {
                ++it1;
                ++it2;
            }
        }
    }

    return 0;
}

template<typename Iterator1, typename Iterator2>
bool equal_n(Iterator1 it1, Iterator2 it2, size_t n){
    return compare_n(it1, it2, n) == 0;
}

template<typename T>
constexpr const T& min(const T& a, const T& b){
    return a <= b ? a : b;
}

template<typename T>
constexpr const T& max(const T& a, const T& b){
    return a >= b ? a : b;
}

} //end of namespace std

#endif
