//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MEMORY_H
#define MEMORY_H

#include "enable_if.hpp"
#include "utility.hpp"

namespace std {

template< class T >
T* addressof(T& arg){
    return reinterpret_cast<T*>(
        &const_cast<char&>(
        reinterpret_cast<const volatile char&>(arg)));
}

template <typename T>
struct has_overloaded_addressof {
    template <class X>
    static constexpr bool has_overload(...) {
        return false;
    }

    template <class X, size_t S = sizeof(std::declval<X&>().operator&()) >
    static constexpr bool has_overload(bool) {
        return true;
    }

    constexpr static bool value = has_overload<T>(true);
};

template <typename T>
constexpr typename std::disable_if_t<has_overloaded_addressof<T>::value, T*>
static_addressof(T& ref){
  return &ref;
}

template <typename T>
constexpr typename std::enable_if_t<has_overloaded_addressof<T>::value, T*>
static_addressof(T& ref){
  return std::addressof(ref);
}

} //end of namespace std

#endif
