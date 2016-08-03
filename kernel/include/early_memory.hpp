//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
 * This header contains addresses where the init stage of the kernel writes information for the later stage
 */

#ifndef EARLY_MEMORY_H
#define EARLY_MEMORY_H

// The address of the kernel
constexpr const uint32_t kernel_address = 0x100000;  //1Mib aligned size (kernel_mib)

// The first address used for early memory
constexpr const uint32_t early_base = 0x90000;

// The number of MiB used by the kernel
constexpr const uint32_t kernel_mib_address = 0x90000; //4 bytes (32 bits)

inline uint32_t kernel_mib(){
    return *reinterpret_cast<uint32_t*>(kernel_mib_address);
}

inline void kernel_mib(uint32_t value){
    *reinterpret_cast<uint32_t*>(kernel_mib_address) = value;
}

// Maximum number of early logs
constexpr const uint32_t MAX_EARLY_LOGGING = 128;

//TODO Find a way for this shit to works in boot-16
constexpr const uint32_t early_logs_count_address = 0x90004; // 4 bytes (32 bits)
constexpr const uint32_t early_logs_address = 0x90008; // 128 * 4 bytes (32 bits ) = 512

inline uint32_t early_logs_count(){
    return *reinterpret_cast<uint32_t*>(early_logs_count_address);
}

inline void early_logs_count(uint32_t value){
    *reinterpret_cast<uint32_t*>(early_logs_count_address) = value;
}

inline uint32_t* early_logs(){
    return *reinterpret_cast<uint32_t**>(early_logs_address);
}

constexpr const uint32_t e820_entry_count_address = 0x90208; // 4 bytes (32 bits)

inline uint32_t e820_entry_count(){
    return *reinterpret_cast<uint32_t*>(e820_entry_count_address);
}

inline void e820_entry_count(uint32_t value){
    *reinterpret_cast<uint32_t*>(e820_entry_count_address) = value;
}

#endif
