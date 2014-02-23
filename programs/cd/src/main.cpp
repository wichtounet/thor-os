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
        print_line("Usage: cd file_path");
        exit(1);
    }

    std::string path(argv[1]);

    auto fd = open(path.c_str());

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            if(!(info->flags & STAT_FLAG_DIRECTORY)){
                print_line("cat: error: Is not a directory");
            } else {
                auto cwd = current_working_directory();

                if(path[0] == '/'){
                    cwd = "/";
                }

                auto parts = std::split(path, '/');
                for(auto& part : parts){
                    cwd += part;
                    cwd += '/';
                }

                set_current_working_directory(cwd);
            }
        } else {
            printf("cd: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("cd: error: %s\n", std::error_message(fd.error()));
    }

    exit(0);
}