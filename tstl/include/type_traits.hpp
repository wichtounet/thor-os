//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

#include "enable_if.hpp"

namespace std {

template<typename T>
struct is_trivially_destructible {
    static constexpr const bool value = __has_trivial_destructor(T);
};

template <typename T>
struct is_reference {
    static constexpr const bool value = false;
};

template <typename T>
struct is_reference<T&>{
    static constexpr const bool value = true;
};

template <typename T>
struct is_reference<T&&>{
    static constexpr const bool value = true;
};

template<typename T, typename Enable = void>
struct add_rvalue_reference;

template<typename T>
struct add_rvalue_reference<T, typename std::enable_if<std::is_reference<T>::value>::type> {
    typedef T type;
};

template<typename T>
struct add_rvalue_reference<T, typename std::enable_if<!std::is_reference<T>::value>::type> {
    typedef T&& type;
};

} //end of namespace std

#endif
