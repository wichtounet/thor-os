//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef UTILITY_H
#define UTILITY_H

#include "type_traits.hpp"

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

template <typename T>
void swap (T& a, T& b){
    T c(std::move(a));
    a = std::move(b);
    b = std::move(c);
}

template<typename T>
typename std::add_rvalue_reference<T>::type declval();

} //end of namespace std

#endif
