//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

/*
 * This header contains addresses where the init stage of the kernel writes information for the later stage
 */

#ifndef EARLY_MEMORY_H
#define EARLY_MEMORY_H

namespace early {

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

constexpr const uint32_t early_logs_count_address = 0x90004; // 4 bytes (32 bits)
constexpr const uint32_t early_logs_address = 0x90008; // 128 * 4 bytes (32 bits ) = 512

inline uint32_t early_logs_count(){
    return *reinterpret_cast<uint32_t*>(early_logs_count_address);
}

inline void early_logs_count(uint32_t value){
    *reinterpret_cast<uint32_t*>(early_logs_count_address) = value;
}

constexpr const uint32_t e820_entry_count_address = 0x90208; // 4 bytes (32 bits)
constexpr const uint32_t e820_entry_address = 0x9020C; // 20 * 20 bytes

inline uint32_t e820_entry_count(){
    return *reinterpret_cast<uint32_t*>(e820_entry_count_address);
}

inline void e820_entry_count(uint32_t value){
    *reinterpret_cast<uint32_t*>(e820_entry_count_address) = value;
}

constexpr const uint32_t vesa_enabled_address = 0x9039C;   // 4 bytes (32 bits)
constexpr const uint32_t vesa_mode_info_address = 0x903A0; // 256 bytes

inline bool vesa_enabled(){
    return static_cast<bool>(*reinterpret_cast<uint32_t*>(vesa_enabled_address));
}

inline void vesa_enabled(bool value){
    *reinterpret_cast<uint32_t*>(vesa_enabled_address) = value ? 1 : 0;
}

constexpr const uint32_t tss_address = 0x904A0; // 104 bytes (aligned to 128)

} // end of namespace early

#endif
