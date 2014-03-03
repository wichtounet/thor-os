//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
