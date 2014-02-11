//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <system.hpp>

void exit(size_t return_code) {
    asm volatile("mov rax, 0x666; mov rbx, %[ret]; int 50"
        : //No outputs
        : [ret] "g" (return_code)
        : "rax", "rbx");

    __builtin_unreachable();
}

std::expected<size_t> exec(const char* executable){
    int64_t pid;
    //__asm__ __volatile__("xchg bx, bx");
    asm volatile("mov rax, 5; mov rbx, %[path]; int 50; mov %[pid], rax"
        : [pid] "=m" (pid)
        : [path] "g" (reinterpret_cast<size_t>(executable))
        : "rax", "rbx");
    //__asm__ __volatile__("xchg bx, bx");

    if(pid < 0){
        return std::make_expected_from_error<size_t, size_t>(-pid);
    } else {
        return std::make_expected<size_t>(pid);
    }
}

void await_termination(size_t pid) {
    asm volatile("mov rax, 6; mov rbx, %[pid]; int 50;"
        : //No outputs
        : [pid] "g" (pid)
        : "rax", "rbx");
}

void sleep_ms(size_t ms){
    asm volatile("mov rax, 4; mov rbx, %[ms]; int 50"
        : //No outputs
        : [ms] "g" (ms)
        : "rax", "rbx");
}

std::expected<size_t> exec_and_wait(const char* executable){
    auto result = exec(executable);

    if(result.valid()){
        await_termination(result.value());
    }

    return std::move(result);
}
