//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef THOR_H
#define THOR_H

#include "memory.hpp"

void* operator new (uint64_t size);
void operator delete (void *p);

#endif
