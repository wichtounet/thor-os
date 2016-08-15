//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <file.hpp>
#include <system.hpp>
#include <errors.hpp>
#include <print.hpp>
#include <elf.hpp>

namespace {

void readelf(char* buffer){
    if(!elf::is_valid(buffer)){
        print_line("readelf: This file is not an ELF file or not in ELF64 format");

        return;
    }

    auto header = reinterpret_cast<elf::elf_header*>(buffer);

    printf("%h\n", reinterpret_cast<size_t>(buffer));

    printf("Number of Program Headers: %u\n", static_cast<uint64_t>(header->e_phnum));
    printf("Number of Section Headers: %u\n", static_cast<uint64_t>(header->e_shnum));

    auto program_header_table = reinterpret_cast<elf::program_header*>(buffer + header->e_phoff);
    auto section_header_table = reinterpret_cast<elf::section_header*>(buffer + header->e_shoff);

    auto& string_table_header = section_header_table[header->e_shstrndx];
    auto string_table = buffer + string_table_header.sh_offset;

    for(size_t p = 0; p < header->e_phnum; ++p){
        auto& p_header = program_header_table[p];

        printf("Program header %u\n", p);
        printf("\tVirtual Address: %h\n", p_header.p_paddr);
        printf("\tMSize: %u\t", p_header.p_memsz);
        printf("\tFSize: %u\t Offset: %u \n", p_header.p_filesize, p_header.p_offset);
    }

    for(size_t s = 0; s < header->e_shnum; ++s){
        auto& s_header = section_header_table[s];

        printf("Section \"%s\" (", &string_table[s_header.sh_name]);

        if(s_header.sh_flags & 0x1){
            print(" W");
        }

        if(s_header.sh_flags & 0x2){
            print(" A");
        }

        if(s_header.sh_flags & 0x4){
            print(" X");
        }

        if(s_header.sh_flags & 0x0F000000){
            print(" OS");
        }

        if(s_header.sh_flags & 0xF0000000){
            print(" CPU");
        }
        print_line(")");
        printf("\tAddress: %h Size: %u Offset: %u\n", s_header.sh_addr, s_header.sh_size, s_header.sh_offset);
    }
}

} //end of anonymous namespace

int main(int argc, char* argv[]){
    if(argc == 1){
        print_line("Usage: readelf file_path");
        return 1;
    }

    auto fd = open(argv[1]);

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            if(info->flags & STAT_FLAG_DIRECTORY){
                print_line("readelf: error: Is a directory");
            } else {
                auto size = info->size;

                if(size == 0){
                    print_line("readelf: error: The file is empty");
                } else {
                    auto buffer = new char[size];

                    auto content_result = read(*fd, buffer, size);

                    if(content_result.valid()){
                        if(*content_result != size){
                            //TODO Read more
                        } else {
                            readelf(buffer);
                        }
                    } else {
                        printf("readelf: error: %s\n", std::error_message(content_result.error()));
                    }

                    delete[] buffer;
                }
            }
        } else {
            printf("readelf: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("readelf: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}