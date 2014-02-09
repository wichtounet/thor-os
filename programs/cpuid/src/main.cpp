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

void get_base_info(){
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    native_cpuid(1, &eax, &ebx, &ecx, &edx);

    printf("Stepping: %u\n", eax & 0xF);
    printf("Model: %u\n", (eax >> 4) & 0xF);
    printf("Family: %u\n", (eax >> 8) & 0xF);
    printf("Processor Type: %u\n", (eax >> 12) & 0x3);
    printf("Extended Model: %u\n", (eax >> 16) & 0xF);
    printf("Extended Family: %u\n", (eax >> 20) & 0xFF);
}

} //end of anonymous namespace

int main(){
    get_base_info();

    exit(0);
}