//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <io.hpp>

int64_t ioctl(size_t device, ioctl_request request, void* data){
    int64_t code;
    asm volatile("mov rax, 0x2000; mov rbx, %[device]; mov rcx, %[request]; mov rdx, %[data]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [device] "g" (device), [request] "g" (static_cast<size_t>(request)), [data] "g" (reinterpret_cast<size_t>(data))
        : "rax", "rbx", "rcx", "rdx");
    return code;
}
