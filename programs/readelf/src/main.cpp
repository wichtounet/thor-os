//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/file.hpp>
#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/elf.hpp>

namespace {

void readelf(char* buffer){
    if(!elf::is_valid(buffer)){
        tlib::print_line("readelf: This file is not an ELF file or not in ELF64 format");

        return;
    }

    auto header = reinterpret_cast<elf::elf_header*>(buffer);

    tlib::printf("%h\n", reinterpret_cast<size_t>(buffer));

    tlib::printf("Number of Program Headers: %u\n", static_cast<uint64_t>(header->e_phnum));
    tlib::printf("Number of Section Headers: %u\n", static_cast<uint64_t>(header->e_shnum));

    auto program_header_table = reinterpret_cast<elf::program_header*>(buffer + header->e_phoff);
    auto section_header_table = reinterpret_cast<elf::section_header*>(buffer + header->e_shoff);

    auto& string_table_header = section_header_table[header->e_shstrndx];
    auto string_table = buffer + string_table_header.sh_offset;

    for(size_t p = 0; p < header->e_phnum; ++p){
        auto& p_header = program_header_table[p];

        tlib::printf("Program header %u\n", p);
        tlib::printf("\tVirtual Address: %h\n", p_header.p_paddr);
        tlib::printf("\tMSize: %u\t", p_header.p_memsz);
        tlib::printf("\tFSize: %u\t Offset: %u \n", p_header.p_filesize, p_header.p_offset);
    }

    for(size_t s = 0; s < header->e_shnum; ++s){
        auto& s_header = section_header_table[s];

        tlib::printf("Section \"%s\" (", &string_table[s_header.sh_name]);

        if(s_header.sh_flags & 0x1){
            tlib::print(" W");
        }

        if(s_header.sh_flags & 0x2){
            tlib::print(" A");
        }

        if(s_header.sh_flags & 0x4){
            tlib::print(" X");
        }

        if(s_header.sh_flags & 0x0F000000){
            tlib::print(" OS");
        }

        if(s_header.sh_flags & 0xF0000000){
            tlib::print(" CPU");
        }
        tlib::print_line(")");
        tlib::printf("\tAddress: %h Size: %u Offset: %u\n", s_header.sh_addr, s_header.sh_size, s_header.sh_offset);
    }
}

} //end of anonymous namespace

int main(int argc, char* argv[]){
    if(argc == 1){
        tlib::print_line("Usage: readelf file_path");
        return 1;
    }

    auto fd = tlib::open(argv[1]);

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            if(info->flags & tlib::STAT_FLAG_DIRECTORY){
                tlib::print_line("readelf: error: Is a directory");
            } else {
                auto size = info->size;

                if(size == 0){
                    tlib::print_line("readelf: error: The file is empty");
                } else {
                    auto buffer = new char[size];

                    auto content_result = tlib::read(*fd, buffer, size);

                    if(content_result.valid()){
                        if(*content_result != size){
                            //TODO Read more
                        } else {
                            readelf(buffer);
                        }
                    } else {
                        tlib::printf("readelf: error: %s\n", std::error_message(content_result.error()));
                    }

                    delete[] buffer;
                }
            }
        } else {
            tlib::printf("readelf: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("readelf: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
