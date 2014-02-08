//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USERLIB_MALLOC_H
#define USERLIB_MALLOC_H

#include "types.hpp"

void* malloc(size_t size);
void free(void* pointer);

size_t brk_start();
size_t brk_end();
size_t sbrk(size_t inc);

#endif
