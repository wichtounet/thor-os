//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef MMAP_H
#define MMAP_H

#include <types.hpp>

void* mmap_phys(size_t phys, size_t size);
bool munmap_phys(void* virt, size_t size);

#endif
