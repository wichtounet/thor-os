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
#include <mount_point.hpp>
#include <statfs_info.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

int main(int argc, char* argv[]){
    bool human = false;

    for(size_t i = 1; i < argc; ++i){
        std::string param(argv[i]);

        if(param == "-h"){
            human = true;
        }
    }

    auto buffer = new char[BUFFER_SIZE];

    auto mp_result = mounts(buffer, BUFFER_SIZE);

    if(mp_result.valid()){
        size_t position = 0;

        print_line("File system Size Used Available");

        while(true){
            auto entry = reinterpret_cast<mount_point*>(buffer + position);

            auto mount_point = &entry->name;

            auto statfs_result = statfs(mount_point);

            if(statfs_result.valid()){
                auto& statfs = *statfs_result;

                if(human){
                    printf("%s %m %m %m\n", mount_point, statfs.total_size, statfs.total_size - statfs.free_size, statfs.free_size);
                } else {
                    printf("%s %u %u %u\n", mount_point, statfs.total_size, statfs.total_size - statfs.free_size, statfs.free_size);
                }
            } else {
                printf("df: error: %s\n", std::error_message(statfs_result.error()));
            }

            if(!entry->offset_next){
                break;
            }

            position += entry->offset_next;
        }
    } else {
        printf("df: error: %s\n", std::error_message(mp_result.error()));
    }

    delete[] buffer;

    exit(0);
}