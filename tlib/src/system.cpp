//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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

std::expected<size_t> exec(const char* executable, const std::vector<std::string>& params){
    const char** args = nullptr;
    if(!params.empty()){
        args = new const char*[params.size()];

        for(size_t i = 0; i < params.size(); ++i){
            args[i] = params[i].c_str();
        }
    }

    int64_t pid;
    asm volatile("mov rax, 5; mov rbx, %[path]; mov rcx, %[argc]; mov rdx, %[argv]; int 50; mov %[pid], rax"
        : [pid] "=m" (pid)
        : [path] "g" (reinterpret_cast<size_t>(executable)), [argc] "g" (params.size()), [argv] "g" (reinterpret_cast<size_t>(args))
        : "rax", "rbx", "rcx", "rdx");

    if(args){
        delete[] args;
    }

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

datetime local_date(){
    datetime date_s;

    asm volatile("mov rax, 400; mov rbx, %[buffer]; int 50; "
        : /* No outputs */
        : [buffer] "g" (reinterpret_cast<size_t>(&date_s))
        : "rax", "rbx");

    return date_s;
}

std::expected<size_t> exec_and_wait(const char* executable, const std::vector<std::string>& params){
    auto result = exec(executable, params);

    if(result.valid()){
        await_termination(result.value());
    }

    return std::move(result);
}

void reboot(){
    asm volatile("mov rax, 201; int 50"
        : //No outputs
        : //No inputs
        : "rax");

    __builtin_unreachable();
}

void shutdown(){
    asm volatile("mov rax, 202; int 50"
        : //No outputs
        : //No inputs
        : "rax");

    __builtin_unreachable();
}
