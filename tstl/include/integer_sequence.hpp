//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TSTL_INTEGER_SEQUENCE_H
#define TSTL_INTEGER_SEQUENCE_H

#include<types.hpp>
#include<enable_if.hpp>

namespace std {

template<typename T, T... Values>
struct integer_sequence {
    static constexpr size_t size() noexcept {
        return sizeof...(Values);
    }
};

template <size_t... Values>
using index_sequence = integer_sequence<size_t, Values...>;

template <typename, size_t, bool>
struct sequence_concat_impl;

template <typename T, T... I, size_t N>
struct sequence_concat_impl<integer_sequence<T, I...>, N, false> {
    using type = integer_sequence<T, I..., (N + I)...>;
};

template <typename T, T... I, size_t N>
struct sequence_concat_impl<integer_sequence<T, I...>, N, true> {
    using type = integer_sequence<T, I..., (N + I)..., 2 * N>;
};

// The 0 and 1 cannot be deduced directly, must use SFINAE

// Base type for generating a sequence
template <typename T, T N, typename Enable = void>
struct make_integer_sequence_impl;

// The general case construct a list by concatenating
template <typename T, T N, typename Enable>
struct make_integer_sequence_impl {
    using type = typename sequence_concat_impl<typename make_integer_sequence_impl<T, N / 2>::type, N / 2, N % 2 == 1>::type;
};

// Specialization for empty sequence
template <typename T, T N>
struct make_integer_sequence_impl<T, N, std::enable_if_t<(N == 0)>> {
    using type = integer_sequence<T>;
};

// Specialization for sequence of length one
template <typename T, T N>
struct make_integer_sequence_impl<T, N, std::enable_if_t<(N == 1)>> {
    using type = integer_sequence<T, 0>;
};

template <typename T, T N>
using make_integer_sequence = typename make_integer_sequence_impl<T, N>::type;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

} //end of namespace std

#endif
