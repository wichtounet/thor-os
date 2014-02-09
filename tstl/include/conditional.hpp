//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONDITIONAL_H
#define CONDITIONAL_H

namespace std {

template<bool B, typename If, typename Else>
struct conditional {};

template<typename If, typename Else>
struct conditional<true, If, Else> { typedef If type; };

template<typename If, typename Else>
struct conditional<false, If, Else> { typedef Else type; };

} //end of namespace std

#endif
