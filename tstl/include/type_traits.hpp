//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
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
    using value_type      = typename T::value_type; ///< The value type of the iterator
    using reference       = typename T::reference; ///< The reference type of the iterator
    using pointer         = typename T::pointer; ///< The pointer type of the iterator
    using difference_type = typename T::difference_type; ///< The difference type of the iterator
};

template <typename T>
struct iterator_traits <T*> {
    using value_type      = T; ///< The value type of the iterator
    using reference       = T&; ///< The reference type of the iterator
    using pointer         = T*; ///< The pointer type of the iterator
    using difference_type = size_t; ///< The difference type of the iterator
};

/* integral_constant */

template <typename T, T V>
struct integral_constant {
    static constexpr T value = V;

    using value_type = T;
    using type       = integral_constant<T, V>;

    constexpr operator value_type() const noexcept {
        return value;
    }

    constexpr value_type operator()() const noexcept {
        return value;
    }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

/* declval */

template <typename T, typename _Up = T&&>
_Up declval_impl(int);

template <typename T>
T declval_impl(long);

template <typename T>
auto declval() noexcept -> decltype(declval_impl<T>(0));

/* remove_reference */

template <typename T>
struct remove_reference {
    using type = T;
};

template<typename T>
struct remove_reference<T&>{
    using type = T;
};

template<typename T>
struct remove_reference<T&&> {
    using type = T;
};

template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

/* remove_extent */

template<typename T>
struct remove_extent {
    using type = T;
};

template<typename T>
struct remove_extent<T[]> {
    using type = T;
};

template<typename T, size_t N>
struct remove_extent<T[N]> {
    using type = T;
};

template<typename T>
using remove_extent_t = typename remove_extent<T>::type;

/* remove_const */

template<typename T>
struct remove_const {
    using type = T;
};

template<typename T>
struct remove_const<const T> {
    using type = T;
};

template<typename T>
using remove_const_t = typename remove_const<T>::type;

/* add_const */

template<typename T>
struct add_const {
    using type = const T;
};

template<typename T>
struct add_const<const T> {
    using type = T;
};

template<typename T>
using add_const_t = typename add_const<T>::type;

/* remove_volatile */

template<typename T>
struct remove_volatile {
    using type = T;
};

template<typename T>
struct remove_volatile<volatile T> {
    using type = T;
};

template<typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

/* remove_cv */

template<typename T>
struct remove_cv {
    using type = typename std::remove_volatile<typename std::remove_const<T>::type>::type;
};

template<typename T>
using remove_cv_t = typename remove_cv<T>::type;

// is_void */

template <typename>
struct is_void_impl : false_type {};

template <>
struct is_void_impl<void> : true_type {};

template <typename _Tp>
struct is_void : is_void_impl<remove_cv_t<_Tp>>::type {};

/* conditional */

template<bool B, typename T, typename F>
struct conditional {
    using type = T;
};

template<typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};

template<bool B, typename T, typename F>
using conditional_t = typename std::conditional<B, T, F>::type;

/* is_trivially_destructible */

template<typename T>
struct is_trivially_destructible {
    static constexpr const bool value = __has_trivial_destructor(T);
};

template<>
struct is_trivially_destructible<void> {
    static constexpr const bool value = true;
};

/* is_trivially_destructible */

template<typename T>
struct has_trivial_assign {
    static constexpr const bool value = __has_trivial_assign(T);
};

/* is_pointer */

/*!
 * \brief Traits to test if given type is a pointer type
 */
template <typename T>
struct is_pointer {
    static constexpr const bool value = false;
};

/*!
 * \copdoc is_pointer
 */
template <typename T>
struct is_pointer<T*>{
    static constexpr const bool value = true;
};

/* is_reference */

/*!
 * \brief Traits to test if given type is a reference type
 */
template <typename T>
struct is_reference {
    static constexpr const bool value = false;
};

/*!
 * \copdoc is_reference
 */
template <typename T>
struct is_reference<T&>{
    static constexpr const bool value = true;
};

/*!
 * \copdoc is_reference
 */
template <typename T>
struct is_reference<T&&>{
    static constexpr const bool value = true;
};

/* is_array */

/*!
 * \brief Traits to test if given type is an array type
 */
template<typename T>
struct is_array {
    static constexpr const bool value = false;
};

/*!
 * \copdoc is_array
 */
template<typename T>
struct is_array<T[]>{
    static constexpr const bool value = true;
};

/*!
 * \copdoc is_array
 */
template<typename T, size_t N>
struct is_array<T[N]>{
    static constexpr const bool value = true;
};

/* is_function */

template<typename T>
struct is_function {
    static constexpr const bool value = false;
};

template<typename Ret, typename... Args>
struct is_function<Ret(Args...)> {
    static constexpr const bool value = true;
};

template<typename Ret, typename... Args>
struct is_function<Ret(Args......)> {
    static constexpr const bool value = true;
};

/* add_rvalue_reference */

template<typename T, typename Enable = void>
struct add_rvalue_reference;

template<typename T>
struct add_rvalue_reference<T, typename std::enable_if_t<std::is_reference<T>::value>> {
    using type = T;
};

template<typename T>
struct add_rvalue_reference<T, typename std::disable_if_t<!std::is_reference<T>::value>> {
    using type = T&&;
};

/* add_pointer */

template<typename T>
struct add_pointer {
    using type = typename std::remove_reference<T>::type*;
};

template<typename T>
using add_pointer_t = typename add_pointer<T>::type;

/* decay */

template<typename T>
struct decay {
    using U    = std::remove_reference_t<T>;
    using type = std::conditional_t<
        std::is_array<U>::value,
        std::remove_extent_t<U>*,
        std::conditional_t<
            std::is_function<U>::value,
            std::add_pointer_t<U>,
            std::remove_cv_t<U>>>;
};

/* is_same */

/*!
 * \brief Traits to test if two types are the same
 */
template<typename T1, typename T2>
struct is_same {
    static constexpr const bool value = false;
};

/*!
 * \copydoc is_same
 */
template<typename T1>
struct is_same <T1, T1> {
    static constexpr const bool value = true;
};

/* is_integral */

template <typename>
struct is_integral {
    static constexpr const bool value = false;
};

template <typename T>
struct is_integral <const T>{
    static constexpr const bool value = is_integral<T>::value;
};

template <>
struct is_integral <bool> {
    static constexpr const bool value = true;
};

template <>
struct is_integral <char> {
    static constexpr const bool value = true;
};

template <>
struct is_integral <signed char> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<unsigned char> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<short> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<unsigned short> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<int> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<unsigned int> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<long> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<unsigned long> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<long long> {
    static constexpr const bool value = true;
};

template <>
struct is_integral<unsigned long long> {
    static constexpr const bool value = true;
};

template <typename T>
struct identity_of {
    using type = T;
};

template <typename T>
using identity_of_t = typename identity_of<T>::type;

// is_convertible

template <typename From, typename To, bool = is_void<From>::value || is_function<To>::value || is_array<To>::value>
struct is_convertible_impl {
    using type = typename is_void<To>::type;
};

template <typename From, typename To>
struct is_convertible_impl<From, To, false> {
private:
    template <typename To1>
    static void test_aux(To1);

    template <typename From1, typename To1, typename = decltype(test_aux<To1>(std::declval<From1>()))>
    static true_type test(int);

    template <typename, typename>
    static false_type test(...);

public:
    using type = decltype(test<From, To>(0));
};

// is_convertible
template <typename From, typename To>
struct is_convertible : is_convertible_impl<From, To>::type {};

} //end of namespace std

#endif
