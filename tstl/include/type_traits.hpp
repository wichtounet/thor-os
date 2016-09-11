//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

#include <types.hpp>
#include <enable_if.hpp>

namespace std {

template <typename T>
struct iterator_traits {
    using value_type      = typename T::value_type;
    using reference       = typename T::reference;
    using pointer         = typename T::pointer;
    using difference_type = typename T::difference_type;
};

template <typename T>
struct iterator_traits <T*> {
    using value_type      = T;
    using reference       = T&;
    using pointer         = T*;
    using difference_type = size_t;
};

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

template<bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

/* is_trivially_destructible */

template<typename T>
struct is_trivially_destructible {
    static constexpr const bool value = __has_trivial_destructor(T);
};

/* is_trivially_destructible */

template<typename T>
struct has_trivial_assign {
    static constexpr const bool value = __has_trivial_assign(T);
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

/* is_same */

template<typename T1, typename T2>
struct is_same {
    static constexpr const bool value = false;
};

template<typename T1>
struct is_same <T1, T1> {
    static constexpr const bool value = true;
};

} //end of namespace std

#endif
