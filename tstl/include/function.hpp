//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STD_FUNCTION_HPP
#define STD_FUNCTION_HPP

#include <algorithms.hpp>
#include <type_traits.hpp>

namespace std {

template<typename>
struct function;

template<typename R, typename... Args>
struct function<R(Args...)> {
public:
    template<typename T>
    function(T&& t){
        typedef model<typename std::decay<T>::type> impl;

        static_assert(sizeof(impl) <= sizeof(storage), "Limited size in function");
        static_assert(std::is_trivially_destructible<T>::value, "Limited support of function");

        new (&storage) impl(std::forward<T>(t));
    }

    function(const function& rhs) = delete;
    function& operator=(const function& rhs) = delete;

    function(function&& rhs) = delete;
    function& operator=(function&& rhs) = delete;

    R operator()(Args... args) const {
        return (*reinterpret_cast<const concept*>(&storage))(std::forward<Args>(args)...);
    }

private:
    size_t storage[2];

    struct concept {
        virtual R operator()(Args...) const = 0;
    };

    template<typename T>
    struct model : concept {
        template<typename U>
        model(U&& u) : t(std::forward<U>(u)){
            //Nothing to init
        }

        R operator()(Args... args) const override {
            return t(std::forward<Args>(args)...);
        }

        T t;
    };
};

} //end of namespace std

#endif
