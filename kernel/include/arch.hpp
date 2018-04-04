//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ARCH_H
#define ARCH_H

#include <types.hpp>

namespace arch {

void enable_sse();

inline size_t get_rflags(){
    size_t rflags;
    asm volatile("pushfq; pop %0;" : "=g" (rflags));
    return rflags;
}

inline void disable_hwint(size_t& rflags){
    asm volatile("pushfq; pop %0; cli;" : "=g" (rflags));
}

inline void enable_hwint(size_t& rflags){
    asm volatile("push %0; popfq; " :: "g" (rflags));
}

inline bool interrupts_enabled(){
    auto flags = get_rflags();
    return flags & 0x200;
}

} //enf of arch namespace

#endif
