//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/io.hpp"

int64_t tlib::ioctl(size_t device, tlib::ioctl_request request, void* data){
    int64_t code;
    asm volatile("mov rax, 0xA00; mov rbx, %[device]; mov rcx, %[request]; mov rdx, %[data]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [device] "g" (device), [request] "g" (static_cast<size_t>(request)), [data] "g" (reinterpret_cast<size_t>(data))
        : "rax", "rbx", "rcx", "rdx");
    return code;
}
