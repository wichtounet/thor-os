//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

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
