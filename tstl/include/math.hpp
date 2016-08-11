//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MATH_HPP
#define MATH_HPP

namespace std {

template<typename T>
T ceil_divide(T base, T divisor){
    return (base + divisor - 1) / divisor;
}

} //end of namespace std

#endif
