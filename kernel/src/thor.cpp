//=======================================================================
// Copyright Baptiste Wicht 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "thor.hpp"
#include "memory.hpp"

void* operator new(uint64_t size){
    return k_malloc(size);
}

void operator delete(void* p){
    k_free(p);
}

void* operator new[](uint64_t size){
    return k_malloc(size);
}

void operator delete[](void* p){
    return k_free(p);
}
