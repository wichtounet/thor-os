//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include "stl/types.hpp"

namespace physical_allocator {

void early_init();
void init();

size_t early_allocate(size_t pages);
size_t allocate(size_t pages);

} //end of physical_allocator namespace

#endif
