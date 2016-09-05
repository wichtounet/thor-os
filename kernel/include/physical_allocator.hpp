//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include <types.hpp>

namespace physical_allocator {

void early_init();

size_t early_allocate(size_t pages);

void init();
void finalize();

size_t allocate(size_t pages);
void free(size_t address, size_t pages);

size_t available();
size_t allocated();
size_t free();

} //end of physical_allocator namespace

#endif
