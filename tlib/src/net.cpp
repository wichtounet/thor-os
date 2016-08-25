//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tlib/net.hpp"

std::expected<size_t> socket_open(network::socket_domain domain, network::socket_type type, network::socket_protocol protocol){
    int64_t fd;
    asm volatile("mov rax, 0x3000; mov rbx, %[type]; mov rcx, %[type]; mov rdx, %[protocol]; int 50; mov %[fd], rax"
        : [fd] "=m" (fd)
        : [domain] "g" (static_cast<size_t>(domain)), [type] "g" (static_cast<size_t>(type)), [protocol] "g" (static_cast<size_t>(protocol))
        : "rax", "rbx", "rcx", "rdx");

    if(fd < 0){
        return std::make_expected_from_error<size_t, size_t>(-fd);
    } else {
        return std::make_expected<size_t>(fd);
    }
}

void socket_close(size_t fd){
    asm volatile("mov rax, 0x3001; mov rbx, %[fd]; int 50;"
        : /* No outputs */
        : [fd] "g" (fd)
        : "rax", "rbx");
}
