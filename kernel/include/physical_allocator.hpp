//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include "stl/types.hpp"

void init_physical_allocator();

size_t allocate_physical_memory(size_t pages);

#endif
