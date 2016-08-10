//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DRIVER_PIT_H
#define DRIVER_PIT_H

#include <types.hpp>

namespace pit {

bool install();
void remove();

uint64_t counter();

} //end of namespace pit

#endif
