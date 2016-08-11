//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include <type_traits.hpp>
#include <utility.hpp>
#include <types.hpp>
#include <enable_if.hpp>

namespace std {

template<typename Iterator>
size_t distance(Iterator it, Iterator end){
    // For now, we only have random access iterator
    return end - it;
}

} //end of namespace std

#endif
