//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef USERLIB_MALLOC_H
#define USERLIB_MALLOC_H

#include "types.hpp"
#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

void* operator new(uint64_t size);
void operator delete(void* p);

void* operator new[](uint64_t size);
void operator delete[](void* p);

namespace tlib {

void* malloc(size_t size);
void free(void* pointer);

size_t brk_start();
size_t brk_end();
size_t sbrk(size_t inc);

} // end of tlib namespace

#endif
