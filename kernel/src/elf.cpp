//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "elf.hpp"

bool elf::is_valid(const std::string& content){
    auto buffer = content.c_str();
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
