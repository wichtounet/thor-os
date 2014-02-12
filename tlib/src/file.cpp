//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <file.hpp>

std::expected<size_t> open(const char* file){
    int64_t fd;
    asm volatile("mov rax, 300; mov rbx, %[path]; int 50; mov %[fd], rax"
        : [fd] "=m" (fd)
        : [path] "g" (reinterpret_cast<size_t>(file))
        : "rax", "rbx");

    if(fd < 0){
        return std::make_expected_from_error<size_t, size_t>(-fd);
    } else {
        return std::make_expected<size_t>(fd);
    }
}

std::expected<stat_info> stat(size_t fd){
    stat_info info;

    int64_t code;
    asm volatile("mov rax, 301; mov rbx, %[fd]; mov rcx, %[buffer]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(&info))
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<stat_info, size_t>(-code);
    } else {
        return std::make_expected<stat_info>(info);
    }
}
