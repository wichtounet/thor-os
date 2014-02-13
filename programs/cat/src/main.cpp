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

int main(int argc, char* argv[]){
    if(argc == 1){
        print_line("Usage: cat file_path");
        exit(1);
    }

    auto fd = open(argv[1]);

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            auto size = info->size;

            std::string content(size);

            auto content_result = read(*fd, content.c_str(), size);

            if(content_result.valid()){
                if(*content_result != size){
                    //TODO Read mode
                } else {
                    print(content);
                }
            } else {
                printf("cat: error: %s\n", std::error_message(content_result.error()));
            }
        } else {
            printf("cat: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("cat: error: %s\n", std::error_message(fd.error()));
    }

    exit(0);
}