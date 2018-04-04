//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TSTL_NTH_TYPE_H
#define TSTL_NTH_TYPE_H

#include<types.hpp>

namespace std {

template<size_t I, size_t S, typename Head, typename... Tail>
struct nth_type_impl {
    using type = typename nth_type_impl<I+1, S, Tail...>::type;
};

template<size_t I, typename Head, typename... Tail>
struct nth_type_impl<I, I, Head, Tail...> {
    using type = Head;
};

template<size_t I, typename... Tail>
using nth_type_t = typename nth_type_impl<0, I, Tail...>::type;

} //end of namespace std

#endif
