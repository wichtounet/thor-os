//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ENABLE_IF_H
#define ENABLE_IF_H

namespace std {

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };

template< bool B, class T = void >
using enable_if_t = typename enable_if<B, T>::type;

template<bool B, class T = void>
struct disable_if {};

template<class T>
struct disable_if<false, T> { typedef T type; };

template< bool B, class T = void >
using disable_if_t = typename disable_if<B, T>::type;

} //end of namespace std

#endif
