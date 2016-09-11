//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MMAP_H
#define MMAP_H

#include <types.hpp>

void* mmap_phys(size_t phys, size_t size);
bool munmap_phys(void* virt, size_t size);

#endif
