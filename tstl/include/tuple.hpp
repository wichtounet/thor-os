//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TUPLE_H
#define TUPLE_H

#include<types.hpp>
#include<enable_if.hpp>
#include<utility.hpp> // std::forward / std::move
#include<integer_sequence.hpp>
#include<nth_type.hpp>

namespace std {

template <typename... T>
struct tuple;

template <size_t I, typename T>
struct tuple_element;

template <size_t I, typename... T>
struct tuple_element<I, tuple<T...>> {
    using type = nth_type_t<I, T...>;
};

template <size_t I, typename... T>
using tuple_element_t = typename tuple_element<I, T...>::type;

template <size_t I, typename... T>
const tuple_element_t<I, T...>& get(const tuple<T...>& v) noexcept;

template <size_t I, typename... T>
tuple_element_t<I, T...>& get(tuple<T...>& v) noexcept ;

template <size_t I, typename... T>
tuple_element_t<I, T...>&& get(tuple<T...>&& v) noexcept ;

template<size_t I, typename T, typename Enable = void>
struct tuple_value {
    T value;

    constexpr tuple_value(){}

    tuple_value(const T& value) : value(value) {
        //Nothing else to init
    }

    template <typename U>
    tuple_value(U&& value) : value(std::forward<U>(value)) {
        //Nothing else to init
    }

    tuple_value( const tuple_value& ) = default;
    tuple_value( tuple_value&& ) = default;

    template <typename U>
    tuple_value& operator=(U&& value){
        this->value = std::forward<U>(value);

        return *this;
    }

    T& get() noexcept {
        return value;
    }

    const T& get() const noexcept {
        return value;
    }
};

template<size_t I, typename T>
struct tuple_value<I, T, std::enable_if_t<__is_empty(T)>> : private T {
    constexpr tuple_value(){}

    tuple_value(const T& value) : T(value) {
        //Nothing else to init
    }

    template <typename U>
    tuple_value(U&& value) : T(std::forward<U>(value)) {
        //Nothing else to init
    }

    tuple_value( const tuple_value& ) = default;
    tuple_value( tuple_value&& ) = default;

    template <typename U>
    tuple_value& operator=(U&& value){
        static_cast<T&>(*this) = std::forward<U>(value);

        return *this;
    }

    T& get() noexcept {
        return static_cast<T&>(*this);
    }

    const T& get() const noexcept {
        return static_cast<const T&>(*this);
    }
};

using swallow = bool[];

template <typename, typename...>
struct tuple_base;

template <size_t... I, typename... T>
struct tuple_base<std::index_sequence<I...>, T...> : tuple_value<I, T>... {
    constexpr tuple_base() = default;

    template<typename... U, typename = std::enable_if_t<(sizeof...(U) == sizeof...(T))>>
    constexpr explicit tuple_base(U&&... values) : tuple_value<I, T>(std::forward<U>(values))... {
        //Nothing else to init
    }

    template <typename... U>
    constexpr explicit tuple_base(const tuple<U...>& values) : tuple_value<I, T>(get<I>(values))... {
        //Nothing else to init
    }

    template <typename... U>
    constexpr explicit tuple_base(tuple<U...>&& values) : tuple_value<I, T>(std::forward<U>(get<I>(values)))... {
        //Nothing else to init
    }

    tuple_base( const tuple_base& ) = default;
    tuple_base( tuple_base&& ) = default;

    tuple_base& operator=(const tuple_base& v) {
        (void)swallow{(tuple_value<I, T>::operator=(static_cast<tuple_value<I, T>&>(v).get()), true)...};
        return *this;
    }

    tuple_base& operator=(tuple_base&& v) {
        (void)swallow{(tuple_value<I, T>::operator=(std::forward<T>(static_cast<tuple_value<I, T>&>(v).get())), true)...};
        return *this;
    }

    template <typename... U>
    tuple_base& operator=(const tuple<U...>& v) {
        (void)swallow{(tuple_value<I, T>::operator=(get<I>(v)), true)...};
        return *this;
    }

    template <typename... U>
    tuple_base& operator=(tuple<U...>&& v) {
        (void)swallow{(tuple_value<I, T>::operator=(get<I>(std::move(v))), true)...};
        return *this;
    }
};

template <typename... T>
struct tuple {
    using base_t = tuple_base<std::make_index_sequence<sizeof...(T)>, T...>;

    base_t base;

    constexpr tuple() : base(){
        //Nothing else to init
    }

    constexpr tuple(const T&... values) : base(values...) {
        //Nothing else to init
    }

    template<typename... U, typename = std::enable_if_t<(sizeof...(U) == sizeof...(T))>>
    constexpr tuple(U&&... values) : base(std::forward<U>(values)...) {
        //Nothing else to init
    }

    tuple( const tuple& ) = default;
    tuple( tuple&& ) = default;

    template<typename... U>
    constexpr tuple(const tuple<U...>& v) : base(v) {
        //Nothing else to init
    }

    template<typename... U>
    constexpr tuple(tuple<U...>&& v) : base(std::move(v)) {
        //Nothing else to init
    }

    template<typename TT>
    tuple& operator=(TT&& value){
        base = std::forward<TT>(value);

        return *this;
    }
};

template <typename T>
struct tuple_size;

template <typename... T>
struct tuple_size<tuple<T...>> {
    static constexpr const size_t value = sizeof...(T);
};

template <size_t I, typename... T>
const nth_type_t<I, T...>& get(const tuple<T...>& v) noexcept {
    return static_cast<const tuple_value<I, nth_type_t<I, T...>>&>(v.base).get();
}

template <size_t I, typename... T>
nth_type_t<I, T...>& get(tuple<T...>& v) noexcept {
    return static_cast<tuple_value<I, nth_type_t<I, T...>>&>(v.base).get();
}

template <size_t I, typename... T>
nth_type_t<I, T...>&& get(tuple<T...>&& v) noexcept {
    return static_cast<nth_type_t<I, T...>&&>(static_cast<tuple_value<I, nth_type_t<I, T...>>&&>(v.base).get());
}

template<typename... E>
inline tuple<E...> make_tuple(E&&... args){
    return tuple<E...>(std::forward<E>(args)...);
}

template< class... Types >
constexpr tuple<Types&...> tie( Types&... args ){
    return tuple<Types&...>(args...);
}

} //end of namespace std

#endif
