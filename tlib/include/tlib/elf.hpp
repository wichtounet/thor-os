//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef ELF_H
#define ELF_H

#include <types.hpp>
#include <string.hpp>

namespace elf {

struct elf_header {
    char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
}__attribute__((packed));

struct program_header {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesize;
    uint64_t p_memsz;
    uint64_t p_align;
}__attribute__((packed));

struct section_header {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
}__attribute__((packed));

inline bool is_valid(const char* buffer){
    auto header = reinterpret_cast<const elf::elf_header*>(buffer);

    //Test if ELF file
    if(header->e_ident[0] == 0x7F && header->e_ident[1] == 'E' &&
        header->e_ident[2] == 'L' && header->e_ident[3] == 'F'){

        //Test if ELF64
        if(header->e_ident[4] == 2){
            return true;
        }
    }

    return false;
}

} //end of namespace elf

#endif
