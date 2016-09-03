//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tlib/net.hpp"
#include "tlib/malloc.hpp"

std::expected<size_t> tlib::socket_open(socket_domain domain, socket_type type, socket_protocol protocol){
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

void tlib::socket_close(size_t fd){
    asm volatile("mov rax, 0x3001; mov rbx, %[fd]; int 50;"
        : /* No outputs */
        : [fd] "g" (fd)
        : "rax", "rbx");
}

std::expected<tlib::packet> tlib::prepare_packet(size_t socket_fd, void* desc){
    auto buffer = malloc(2048);

    int64_t fd;
    uint64_t index;
    asm volatile("mov rax, 0x3002; mov rbx, %[socket]; mov rcx, %[desc]; mov rdx, %[buffer]; int 50; mov %[fd], rax; mov %[index], rbx;"
        : [fd] "=m" (fd), [index] "=m" (index)
        : [socket] "g" (socket_fd), [desc] "g" (reinterpret_cast<size_t>(desc)), [buffer] "g" (reinterpret_cast<size_t>(buffer))
        : "rax", "rbx", "rcx", "rdx");

    if(fd < 0){
        return std::make_expected_from_error<tlib::packet, size_t>(-fd);
    } else {
        tlib::packet p;
        p.fd = fd;
        p.index = index;
        p.payload = static_cast<char*>(buffer);
        return std::make_expected<packet>(p);
    }
}

std::expected<void> tlib::finalize_packet(size_t socket_fd, tlib::packet p){
    auto packet_fd = p.fd;

    int64_t code;
    asm volatile("mov rax, 0x3003; mov rbx, %[socket]; mov rcx, %[packet]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [socket] "g" (socket_fd), [packet] "g" (packet_fd)
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }

    free(p.payload);
}
