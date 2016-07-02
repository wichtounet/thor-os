//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

#include <types.hpp>
#include <enable_if.hpp>

namespace std {

template<class T>
struct remove_reference {
    typedef T type;
};

template<class T>
struct remove_reference<T&>{
    typedef T type;
};

template<class T>
struct remove_reference<T&&> {
    typedef T type;
};

/* remove_extent */

template<class T>
struct remove_extent {
    typedef T type;
};

template<class T>
struct remove_extent<T[]> {
    typedef T type;
};

template<class T, size_t N>
struct remove_extent<T[N]> {
    typedef T type;
};

/* remove_const */

template<class T>
struct remove_const {
    typedef T type;
};

template<class T>
struct remove_const<const T> {
    typedef T type;
};

/* remove_volatile */

template<class T>
struct remove_volatile {
    typedef T type;
};

template<class T>
struct remove_volatile<volatile T> {
    typedef T type;
};

/* remove_cv */

template<class T>
struct remove_cv {
    typedef typename std::remove_volatile<typename std::remove_const<T>::type>::type type;
};

/* conditional */

template<bool B, class T, class F>
struct conditional {
    typedef T type;
};

template<class T, class F>
struct conditional<false, T, F> {
    typedef F type;
};

/* is_trivially_destructible */

template<typename T>
struct is_trivially_destructible {
    static constexpr const bool value = __has_trivial_destructor(T);
};

/* is_reference */

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

/* is_array */

template<class T>
struct is_array {
    static constexpr const bool value = false;
};

template<class T>
struct is_array<T[]>{
    static constexpr const bool value = true;
};

template<class T, size_t N>
struct is_array<T[N]>{
    static constexpr const bool value = true;
};

/* is_function */

template<typename T>
struct is_function {
    static constexpr const bool value = false;
};

template<class Ret, class... Args>
struct is_function<Ret(Args...)> {
    static constexpr const bool value = true;
};

template<class Ret, class... Args>
struct is_function<Ret(Args......)> {
    static constexpr const bool value = true;
};

/* add_rvalue_reference */

template<typename T, typename Enable = void>
struct add_rvalue_reference;

template<typename T>
struct add_rvalue_reference<T, typename std::enable_if_t<std::is_reference<T>::value>> {
    typedef T type;
};

template<typename T>
struct add_rvalue_reference<T, typename std::disable_if_t<!std::is_reference<T>::value>> {
    typedef T&& type;
};

/* add_pointer */

template<typename T>
struct add_pointer {
    typedef typename std::remove_reference<T>::type* type;
};

/* decay */

template<typename T>
struct decay {
    typedef typename std::remove_reference<T>::type U;
    typedef typename std::conditional<
        std::is_array<U>::value,
        typename std::remove_extent<U>::type*,
        typename std::conditional<
            std::is_function<U>::value,
            typename std::add_pointer<U>::type,
            typename std::remove_cv<U>::type
        >::type
    >::type type;
};

} //end of namespace std

#endif
