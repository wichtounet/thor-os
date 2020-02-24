//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include "tlib/system.hpp"

namespace {

uint64_t syscall_get(uint64_t call){
    size_t value;
    asm volatile("mov rax, %[call]; int 50; mov %[value], rax"
        : [value] "=m" (value)
        : [call] "r" (call)
        : "rax");
    return value;
}

} // end of anonymous namespace

void tlib::exit(size_t return_code) {
    asm volatile("mov rax, 0x666; mov rbx, %[ret]; int 50"
        : //No outputs
        : [ret] "g" (return_code)
        : "rax", "rbx");

    __builtin_unreachable();
}

std::expected<size_t> tlib::exec(const char* executable, const std::vector<std::string>& params){
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

void tlib::await_termination(size_t pid) {
    asm volatile("mov rax, 6; mov rbx, %[pid]; int 50;"
        : //No outputs
        : [pid] "g" (pid)
        : "rax", "rbx");
}

void tlib::sleep_ms(size_t ms){
    asm volatile("mov rax, 4; mov rbx, %[ms]; int 50"
        : //No outputs
        : [ms] "g" (ms)
        : "rax", "rbx");
}

tlib::datetime tlib::local_date(){
    tlib::datetime date_s;

    asm volatile("mov rax, 0x400; mov rbx, %[buffer]; int 50; "
        : /* No outputs */
        : [buffer] "g" (reinterpret_cast<size_t>(&date_s))
        : "rax", "rbx");

    return date_s;
}

uint64_t tlib::s_time(){
    return syscall_get(0x401);
}

uint64_t tlib::ms_time(){
    return syscall_get(0x402);
}

std::expected<size_t> tlib::exec_and_wait(const char* executable, const std::vector<std::string>& params){
    auto result = exec(executable, params);

    if(result.valid()){
        await_termination(result.value());
    }

    return std::move(result);
}

void tlib::reboot(unsigned int delay) {
    if (delay) {
        tlib::sleep_ms(1000 * delay);
    }

    asm volatile("mov rax, 0x50; int 50"
        : //No outputs
        : //No inputs
        : "rax");

    __builtin_unreachable();
}

void tlib::shutdown(unsigned int delay) {
    if (delay) {
        tlib::sleep_ms(1000 * delay);
    }

    asm volatile("mov rax, 0x51; int 50"
        : //No outputs
        : //No inputs
        : "rax");

    __builtin_unreachable();
}

void tlib::alpha(){
    asm volatile("mov rax, 0x66; int 50"
        : //No outputs
        : //No inputs
        : "rax");
}
