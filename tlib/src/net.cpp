//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
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

std::expected<void> tlib::listen(size_t socket_fd, bool l){
    int64_t code;
    asm volatile("mov rax, 0x3004; mov rbx, %[socket]; mov rcx, %[listen]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [socket] "g" (socket_fd), [listen] "g" (size_t(l))
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<void, size_t>(-code);
    } else {
        return std::make_expected();
    }
}

std::expected<tlib::packet> tlib::wait_for_packet(size_t socket_fd){
    auto buffer = malloc(2048);

    int64_t code;
    uint64_t payload;
    asm volatile("mov rax, 0x3005; mov rbx, %[socket]; mov rcx, %[buffer]; int 50; mov %[code], rax; mov %[payload], rbx;"
        : [payload] "=m" (payload), [code] "=m" (code)
        : [socket] "g" (socket_fd), [buffer] "g" (reinterpret_cast<size_t>(buffer))
        : "rax", "rbx", "rcx");

    if(code < 0){
        free(buffer);
        return std::make_expected_from_error<packet, size_t>(-code);
    } else {
        tlib::packet p;
        p.index = code;
        p.payload = reinterpret_cast<char*>(payload);
        return std::make_expected<packet>(p);
    }
}

void tlib::release_packet(packet& packet){
    if(packet.payload){
        free(packet.payload);
    }
}
