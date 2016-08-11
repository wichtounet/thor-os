//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <file.hpp>
#include <system.hpp>
#include <errors.hpp>
#include <io.hpp>
#include <print.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("usage: mkfs fs device \n");

        exit(1);
    }

    auto fs_str = argv[1];
    auto device_str = argv[2];

    std::string fs(fs_str);

    if(fs == "fat32"){
        uint64_t size = 0;
        auto code = ioctl(device_str, ioctl_request::GET_BLK_SIZE, &size);

        if(code){
            printf("mkfs: error: %s\n", std::error_message(code));
            exit(1);
        } else {
            printf("mkfs: Creating Fat32 filesystem on %s\n", device_str);
            printf("mkfs: device size: %m\n", size);
        }

        exit(0);
    }

    printf("mkfs: Unsupported filesystem %s\n", fs_str);

    exit(0);
}
