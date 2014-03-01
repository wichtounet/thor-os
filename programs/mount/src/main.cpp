//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <file.hpp>
#include <system.hpp>
#include <errors.hpp>
#include <print.hpp>
#include <mount_point.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

int main(int, char*[]){
    //TODO Add support to mount new directories

    auto buffer = new char[BUFFER_SIZE];

    auto mp_result = mounts(buffer, BUFFER_SIZE);

    if(mp_result.valid()){
        size_t position = 0;

        while(true){
            auto entry = reinterpret_cast<mount_point*>(buffer + position);

            auto mount_point = &entry->name;
            auto device = mount_point + entry->length_mp + 1;
            auto type = device + entry->length_dev + 1;

            printf("%s %s %s\n", mount_point, device, type);

            if(!entry->offset_next){
                break;
            }

            position += entry->offset_next;
        }
    } else {
        printf("mount: error: %s\n", std::error_message(mp_result.error()));
    }

    delete[] buffer;

    exit(0);
}