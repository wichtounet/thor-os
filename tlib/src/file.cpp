//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <file.hpp>

std::expected<size_t> open(const char* file, size_t flags){
    int64_t fd;
    asm volatile("mov rax, 300; mov rbx, %[path]; mov rcx, %[flags]; int 50; mov %[fd], rax"
        : [fd] "=m" (fd)
        : [path] "g" (reinterpret_cast<size_t>(file)), [flags] "g" (flags)
        : "rax", "rbx", "rcx");

    if(fd < 0){
        return std::make_expected_from_error<size_t, size_t>(-fd);
    } else {
        return std::make_expected<size_t>(fd);
    }
}

int64_t mkdir(const char* file){
    int64_t result;
    asm volatile("mov rax, 306; mov rbx, %[path]; int 50; mov %[result], rax"
        : [result] "=m" (result)
        : [path] "g" (reinterpret_cast<size_t>(file))
        : "rax", "rbx");
    return result;
}

int64_t rm(const char* file){
    int64_t result;
    asm volatile("mov rax, 307; mov rbx, %[path]; int 50; mov %[result], rax"
        : [result] "=m" (result)
        : [path] "g" (reinterpret_cast<size_t>(file))
        : "rax", "rbx");
    return result;
}

void close(size_t fd){
    asm volatile("mov rax, 302; mov rbx, %[fd]; int 50;"
        : /* No outputs */
        : [fd] "g" (fd)
        : "rax", "rbx");
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

std::expected<statfs_info> statfs(const char* file){
    statfs_info info;

    int64_t code;
    asm volatile("mov rax, 310; mov rbx, %[path]; mov rcx, %[buffer]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [path] "g" (reinterpret_cast<size_t>(file)), [buffer] "g" (reinterpret_cast<size_t>(&info))
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<statfs_info, size_t>(-code);
    } else {
        return std::make_expected<statfs_info>(info);
    }
}

std::expected<size_t> read(size_t fd, char* buffer, size_t max, size_t offset){
    int64_t code;
    asm volatile("mov rax, 303; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; mov rsi, %[offset]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max), [offset] "g" (offset)
        : "rax", "rbx", "rcx", "rdx", "rsi");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> write(size_t fd, const char* buffer, size_t max, size_t offset){
    int64_t code;
    asm volatile("mov rax, 311; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; mov rsi, %[offset]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max), [offset] "g" (offset)
        : "rax", "rbx", "rcx", "rdx", "rsi");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> truncate(size_t fd, size_t size){
    int64_t code;
    asm volatile("mov rax, 312; mov rbx, %[fd]; mov rcx, %[size]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [size] "g" (size)
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> entries(size_t fd, char* buffer, size_t max){
    int64_t code;
    asm volatile("mov rax, 308; mov rbx, %[fd]; mov rcx, %[buffer]; mov rdx, %[max]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [fd] "g" (fd), [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max)
        : "rax", "rbx", "rcx", "rdx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::expected<size_t> mounts(char* buffer, size_t max){
    int64_t code;
    asm volatile("mov rax, 309; mov rbx, %[buffer]; mov rcx, %[max]; int 50; mov %[code], rax"
        : [code] "=m" (code)
        : [buffer] "g" (reinterpret_cast<size_t>(buffer)), [max] "g" (max)
        : "rax", "rbx", "rcx");

    if(code < 0){
        return std::make_expected_from_error<size_t, size_t>(-code);
    } else {
        return std::make_expected<size_t>(code);
    }
}

std::string current_working_directory(){
    char buffer[128];
    buffer[0] = '\0';

    asm volatile("mov rax, 304; mov rbx, %[buffer]; int 50;"
        : /* No outputs */
        : [buffer] "g" (reinterpret_cast<size_t>(buffer))
        : "rax", "rbx");

    return {buffer};
}

void set_current_working_directory(const std::string& directory){
    asm volatile("mov rax, 305; mov rbx, %[buffer]; int 50;"
        : /* No outputs */
        : [buffer] "g" (reinterpret_cast<size_t>(directory.c_str()))
        : "rax", "rbx");
}
