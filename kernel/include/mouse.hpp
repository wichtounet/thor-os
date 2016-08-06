//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <types.hpp>

namespace mouse {

void install();

uint64_t x();
uint64_t y();

} //end of namespace mouse

#endif
