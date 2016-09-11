//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EXPECTED_H
#define EXPECTED_H

#include "types.hpp"
#include "algorithms.hpp"
#include "type_traits.hpp"
#include "utility.hpp"
#include "memory.hpp"
#include "new.hpp"

namespace std {

constexpr struct only_set_valid_t {} only_set_valid;

template <typename E>
struct exceptional {
    typedef E error_type;

    error_type error;

    exceptional(): error() {
        //Nothing else to init
    }

    explicit exceptional(error_type e) : error(e) {
        //Nothing else to init
    }
};

template<typename T, typename E>
union trivial_expected_storage {
    typedef T value_type;
    typedef E error_type;

    error_type error;
    value_type value;

    constexpr trivial_expected_storage() : value(){
        //Nothing else to init
    }

    constexpr trivial_expected_storage(const exceptional<error_type>& e) : error(e.error){
        //Nothing else to init
    }

    template<typename... Args>
    constexpr trivial_expected_storage(Args&&... args): value(std::forward<Args>(args)...){
        //Nothing else to init
    }

    ~trivial_expected_storage() = default;
};

template<typename T, typename E>
union non_trivial_expected_storage {
    typedef T value_type;
    typedef E error_type;

    error_type error;
    value_type value;

    constexpr non_trivial_expected_storage() : value(){
        //Nothing else to init
    }

    constexpr non_trivial_expected_storage(exceptional<error_type>& e) : error(e.error){
        //Nothing else to init
    }

    template<typename... Args>
    constexpr non_trivial_expected_storage(Args&&... args): value(std::forward<Args>(args)...){
        //Nothing else to init
    }

    ~non_trivial_expected_storage() {};
};

template<typename T, typename E>
struct trivial_expected_base {
    typedef T value_type;
    typedef E error_type;

    bool has_value;
    trivial_expected_storage<T, E> storage;

    trivial_expected_base() : has_value(true), storage(){
        //Nothing else to init
    }

    trivial_expected_base(only_set_valid_t, bool hv) : has_value(hv){
        //Nothing else to init
    }

    trivial_expected_base(const value_type& v) : has_value(true), storage(v){
        //Nothing else to init
    }

    trivial_expected_base(value_type&& v) : has_value(true), storage(std::forward<value_type>(v)){
        //Nothing else to init
    }

    trivial_expected_base(const exceptional<error_type>& e) : has_value(false), storage(e) {
        //Nothing else to init
    }

    ~trivial_expected_base() = default;
};

template<typename T, typename E>
struct non_trivial_expected_base {
    typedef T value_type;
    typedef E error_type;

    bool has_value;
    trivial_expected_storage<T, E> storage;

    non_trivial_expected_base() : has_value(true), storage(){
        //Nothing else to init
    }

    non_trivial_expected_base(only_set_valid_t, bool hv) : has_value(hv){
        //Nothing else to init
    }

    non_trivial_expected_base(const value_type& v) : has_value(true), storage(v){
        //Nothing else to init
    }

    non_trivial_expected_base(value_type&& v) : has_value(true), storage(std::forward<value_type>(v)){
        //Nothing else to init
    }

    non_trivial_expected_base(const exceptional<error_type>& e) : has_value(false), storage(e) {
        //Nothing else to init
    }

    ~non_trivial_expected_base(){
        if(has_value){
            storage.error.~value_type();
        } else {
            storage.error.~error_type();
        }
    }
};

template <typename T, typename E>
using expected_base = typename std::conditional<
        std::is_trivially_destructible<T>::value && std::is_trivially_destructible<E>::value,
        trivial_expected_base<T, E>,
        non_trivial_expected_base<T, E>
    >::type;

template<typename T, typename E = size_t>
struct expected : expected_base<T, E> {
    typedef T value_type;
    typedef E error_type;

    typedef expected<T, E> this_type;
    typedef expected_base<T, E> base_type;

private:
    value_type* value_ptr() {
        return std::addressof(base_type::storage.value);
    }

    constexpr const value_type* value_ptr() const {
        return std::static_addressof(base_type::storage.value);
    }

    error_type* error_ptr() {
        return std::addressof(base_type::storage.error);
    }

    constexpr const error_type* error_ptr() const {
        return static_addressof(base_type::storage.error);
    }

    constexpr const bool& contained_has_value() const& {
        return base_type::has_value;
    }

    bool& contained_has_value() & {
        return base_type::has_value;
    }

    bool&& contained_has_value() && {
        return std::move(base_type::has_value);
    }

    constexpr const value_type& contained_value() const& {
        return base_type::storage.value;
    }

    value_type& contained_value() & {
        return base_type::storage.value;
    }

    value_type&& contained_value() && {
        return std::move(base_type::storage.value);
    }

    constexpr const error_type& contained_error() const& {
        return base_type::storage.error;
    }

    error_type& contained_error() & {
        return base_type::storage.error;
    }

    error_type&& contained_error() && {
        return std::move(base_type::storage.error);
    }

public:
    /* Constructors */

    constexpr expected(const value_type& v) : base_type(v){
        //Nothing else to init
    }

    constexpr expected(value_type&& v) : base_type(std::forward<value_type>(v)){
        //Nothing else to init
    }

    expected(const expected& rhs) : base_type(only_set_valid, rhs.valid()) {
        if(rhs.valid()){
            ::new (value_ptr()) value_type(rhs.contained_value());
        } else {
            ::new (error_ptr()) error_type(rhs.contained_error());
        }
    }

    expected(expected&& rhs) : base_type(only_set_valid, rhs.valid()) {
        if(rhs.valid()){
            new (value_ptr()) value_type(std::move(rhs.contained_value()));
        } else {
            new (error_ptr()) error_type(std::move(rhs.contained_error()));
        }
    }

    expected(const exceptional<error_type>& e) : base_type(e) {
        //Nothing else to init
    }

    expected() : base_type() {}

    ~expected() = default;

    /* Operators */
expected& operator=(const expected& rhs){
        this_type(rhs).swap(*this);
        return *this;
    }

    expected& operator=(expected&& rhs){
        this_type(std::forward(rhs)).swap(*this);
        return *this;
    }

    expected& operator=(const value_type& v){
        this_type(v).swap(*this);
        return *this;
    }

    expected& operator=(value_type&& v){
        this_type(std::forward(v)).swap(*this);
        return *this;
    }

    /* Swap */

    void swap(expected& rhs){
        if (valid()){
            if (rhs.valid()){
                std::swap(contained_value(), rhs.contained_value());
            } else {
                error_type t = std::move(rhs.contained_error());
                new (rhs.value_ptr()) value_type(std::move(contained_value()));
                new (error_ptr()) error_type(t);
                std::swap(contained_has_value(), rhs.contained_has_value());
            }
        } else {
            if (rhs.valid()){
                rhs.swap(*this);
            } else {
                std::swap(contained_error(), rhs.contained_error());
            }
        }
    }

    /* Accessors */

    constexpr bool valid() const {
        return contained_has_value();
    }

    constexpr explicit operator bool() const {
        return valid();
    }

    constexpr const value_type& value() const {
        return contained_value();
    }

    constexpr const value_type&  operator*() const {
        return contained_value();
    }

    value_type& operator*(){
        return contained_value();
    }

    constexpr const value_type* operator->() const {
        return value_ptr();
    }

    value_type* operator->(){
        return value_ptr();
    }

    constexpr const error_type& error() const {
        return contained_error();
    }

    constexpr bool has_error(const error_type& e) const {
        return contained_error() == e;
    }

    constexpr exceptional<error_type> get_exceptional() const {
        return exceptional<error_type>(contained_error());
    }
};

template<typename T>
inline expected<T> make_expected(T&& v){
    return expected<T>(std::forward<T>(v));
}

template<typename T>
inline expected<T> make_expected(const T& v){
    return expected<T>(v);
}

template<typename T, typename U, typename E>
inline expected<T, U> make_expected_from_error(E v){
    return expected<T, U>(exceptional<U>(v));
}

template<typename T, typename E>
inline expected<T, E> make_expected_from_error(E v){
    return expected<T, E>(exceptional<E>(v));
}

} //end of namespace std

#endif
