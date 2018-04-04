//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ATOMIC_HPP
#define ATOMIC_HPP

#include <types.hpp>

namespace std {

template<typename T>
struct atomic;

template<>
struct atomic<bool> {
    using value_type = uint64_t;

    atomic() = default;

    atomic(const atomic& rhs) = delete;
    atomic& operator=(const atomic& rhs) = delete;

    explicit atomic(value_type value) : value(value) {}

    value_type load() const {
        return __atomic_load_n(&value, __ATOMIC_CONSUME);
    }

    value_type operator=(bool new_value){
        __atomic_store_n(&value, new_value, __ATOMIC_RELEASE);

        return new_value;
    }

private:
    volatile bool value;
};

template<>
struct atomic<uint64_t> {
    using value_type = uint64_t;

    atomic() = default;

    atomic(const atomic& rhs) = delete;
    atomic& operator=(const atomic& rhs) = delete;

    explicit atomic(value_type value) : value(value) {}

    value_type load() const {
        return __atomic_load_n(&value, __ATOMIC_CONSUME);
    }

    value_type operator=(uint64_t new_value){
        __atomic_store_n(&value, new_value, __ATOMIC_RELEASE);

        return new_value;
    }

    value_type operator++(){
        auto new_value = __atomic_add_fetch(&value, 1, __ATOMIC_RELEASE);
        return new_value;
    }

    value_type operator++(int){
        auto new_value = __atomic_fetch_add(&value, 1, __ATOMIC_RELEASE);
        return new_value;
    }

private:
    volatile uint64_t value;
};

} //end of namespace std

#endif
