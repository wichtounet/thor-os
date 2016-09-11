//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
 * The implementation of the memory detection is made in boot_16.cpp
 * The finalization of the memory detection is made in e820.cpp once in long
 * mode.
 */

#ifndef E820_HPP
#define E820_HPP

#include "e820_types.hpp"

namespace e820 {

//Must be called by the kernel to transform e820 entries into mmap entries
void finalize_memory_detection();

bool mmap_failed();
uint64_t mmap_entry_count();
const mmapentry& mmap_entry(uint64_t i);
const char* str_e820_type(uint64_t type);

size_t available_memory();

} //end of namespace e820

#endif
