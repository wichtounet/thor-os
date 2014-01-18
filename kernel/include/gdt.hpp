//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GDT_H
#define GDT_H

namespace gdt {

constexpr const uint16_t CODE_SELECTOR = 0x08;
constexpr const uint16_t DATA_SELECTOR = 0x10;
constexpr const uint16_t LONG_SELECTOR = 0x18;
constexpr const uint16_t USER_DATA_SELECTOR = 0x20;
constexpr const uint16_t USER_CODE_SELECTOR = 0x28;
constexpr const uint16_t TSS_SELECTOR = 0x30;

struct gdt_ptr {
    uint16_t length;
    uint32_t pointer;
} __attribute__ ((packed));

struct tss_descriptor_t {
    uint16_t limit_low      : 16;
    uint32_t base_low       : 24;
    uint8_t type            : 4;
    uint8_t always_0_1      : 1;
    uint8_t dpl             : 2;
    uint8_t present         : 1;
    uint8_t limit_high      : 4;
    uint8_t avl             : 1;
    uint8_t always_0_2      : 2;
    uint8_t granularity     : 1;
    uint8_t base_middle     : 8;
    uint32_t base_high      : 32;
    uint8_t reserved_1      : 8;
    uint8_t always_0_3      : 5;
    uint32_t reserved_2     : 19;
} __attribute__((packed));

static_assert(sizeof(tss_descriptor_t) == 16, "TSS descriptor in long mode is 128 bits long");

struct task_state_segment_t {
    uint32_t reserved_0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved_1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved_2;
    uint16_t reserved_3;
    uint16_t io_map_base_address;
};

} //end of namespace gdt

#endif
