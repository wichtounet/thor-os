//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "virtual_allocator.hpp"
#include "paging.hpp"

namespace {

size_t next_virtual_address = 0;
size_t allocated_pages = 0;

} //end of anonymous namespace

void virtual_allocator::init(){
    auto start = paging::virtual_paging_start;
    start += paging::physical_memory_pages * paging::PAGE_SIZE;

    if(start % 0x100000 == 0){
        next_virtual_address = start;
    } else {
        next_virtual_address = (start / 0x100000 + 1) * 0x100000;
    }

    allocated_pages = next_virtual_address / paging::PAGE_SIZE;
}

size_t virtual_allocator::allocate(size_t pages){
    allocated_pages += pages;

    auto address = next_virtual_address;
    next_virtual_address += pages * paging::PAGE_SIZE;
    return address;
}

size_t virtual_allocator::available(){
    return kernel_virtual_size;
}

size_t virtual_allocator::allocated(){
    return allocated_pages * paging::PAGE_SIZE;
}

size_t virtual_allocator::free(){
    return available() - allocated();
}
