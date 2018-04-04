//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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
