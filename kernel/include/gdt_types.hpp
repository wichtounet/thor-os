//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef GDT_TYPES_H
#define GDT_TYPES_H

#include <types.hpp>

namespace gdt {

constexpr const uint16_t CODE_SELECTOR = 0x08;
constexpr const uint16_t DATA_SELECTOR = 0x10;
constexpr const uint16_t LONG_SELECTOR = 0x18;
constexpr const uint16_t USER_CODE_SELECTOR = 0x20;
constexpr const uint16_t USER_DATA_SELECTOR = 0x28;
constexpr const uint16_t TSS_SELECTOR = 0x30;

//Selector types
constexpr const uint16_t SEG_DATA_RD         = 0x00; ///< Read-Only
constexpr const uint16_t SEG_DATA_RDA        = 0x01; ///< Read-Only, accessed
constexpr const uint16_t SEG_DATA_RDWR       = 0x02; ///< Read/Write
constexpr const uint16_t SEG_DATA_RDWRA      = 0x03; ///< Read/Write, accessed
constexpr const uint16_t SEG_DATA_RDEXPD     = 0x04; ///< Read-Only, expand-down
constexpr const uint16_t SEG_DATA_RDEXPDA    = 0x05; ///< Read-Only, expand-down, accessed
constexpr const uint16_t SEG_DATA_RDWREXPD   = 0x06; ///< Read/Write, expand-down
constexpr const uint16_t SEG_DATA_RDWREXPDA  = 0x07; ///< Read/Write, expand-down, accessed
constexpr const uint16_t SEG_CODE_EX         = 0x08; ///< Execute-Only
constexpr const uint16_t SEG_CODE_EXA        = 0x09; ///< Execute-Only, accessed
constexpr const uint16_t SEG_CODE_EXRD       = 0x0A; ///< Execute/Read
constexpr const uint16_t SEG_CODE_EXRDA      = 0x0B; ///< Execute/Read, accessed
constexpr const uint16_t SEG_CODE_EXC        = 0x0C; ///< Execute-Only, conforming
constexpr const uint16_t SEG_CODE_EXCA       = 0x0D; ///< Execute-Only, conforming, accessed
constexpr const uint16_t SEG_CODE_EXRDC      = 0x0E; ///< Execute/Read, conforming
constexpr const uint16_t SEG_CODE_EXRDCA     = 0x0F; ///< Execute/Read, conforming, accessed

constexpr const uint16_t SEG_LDT             = 0x2; ///< LDT
constexpr const uint16_t SEG_TSS_AVAILABLE   = 0x9; ///< 64 bits TSS (Available)
constexpr const uint16_t SEG_TSS_BUSY        = 0xB; ///< 64 bits TSS (Busy)
constexpr const uint16_t SEG_CALL_GATE       = 0xC; ///< 64 bits Call Gate
constexpr const uint16_t SEG_INTERRUPT_GATE  = 0xE; ///< 64 bits Interrupt Gate
constexpr const uint16_t SEG_TRAP_GATE       = 0xE; ///< 64 bits Trap  Gate

struct gdt_ptr {
    uint16_t length;
    uint32_t pointer;
} __attribute__ ((packed));

struct gdt_descriptor_t {
    uint16_t limit_low          : 16;
    uint32_t base_low           : 24;
    uint8_t type                : 4;
    uint8_t always_1            : 1;
    uint8_t dpl                 : 2;
    uint8_t present             : 1;
    uint8_t limit_high          : 4;
    uint8_t avl                 : 1;
    uint8_t long_mode           : 1;
    uint8_t big                 : 1;
    uint8_t granularity         : 1;
    uint32_t base_high          : 8;
} __attribute__((packed));

static_assert(sizeof(gdt_descriptor_t) == 8, "GDT selector in long mode is 8 bytes long");

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
    uint8_t granularity     : 1; uint8_t base_middle     : 8;
    uint32_t base_high      : 32;
    uint8_t reserved_1      : 8;
    uint8_t always_0_3      : 5;
    uint32_t reserved_2     : 19;
} __attribute__((packed));

static_assert(sizeof(tss_descriptor_t) == 16, "TSS descriptor in long mode is 16 bytes long");

struct task_state_segment_t {
    uint32_t reserved_0;
    uint32_t rsp0_low;
    uint32_t rsp0_high;
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
