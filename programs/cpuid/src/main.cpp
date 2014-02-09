//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>

namespace {

void native_cpuid(uint32_t key, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx){
    *eax = key;

    /* ecx is often an input as well as an output. */
    __asm__ __volatile__("cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "0" (*eax), "2" (*ecx));
}

} //end of anonymous namespace

int main(){

    exit(0);
}