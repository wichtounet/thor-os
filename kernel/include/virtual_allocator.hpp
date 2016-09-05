//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef VIRTUAL_ALLOCATOR_H
#define VIRTUAL_ALLOCATOR_H

#include <types.hpp>
#include <literals.hpp>

namespace virtual_allocator {

//The virtual size allocated to the kernel
constexpr const size_t kernel_virtual_size = 1_GiB;

void init();
void finalize();

size_t allocate(size_t pages);
void free(size_t address, size_t pages);

size_t available();
size_t allocated();
size_t free();

} //end of virtual_allocator namespace

#endif
