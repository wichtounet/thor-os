//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef UTILITY_H
#define UTILITY_H

#include "type_traits.hpp"

namespace std {

template <typename T>
void swap (T& a, T& b){
      T c(std::move(a));
      a = std::move(b);
      b = std::move(c);
}

template<typename T>
typename std::add_rvalue_reference<T>::type declval();

} //end of namespace std

#endif
