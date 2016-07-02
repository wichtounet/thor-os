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
#include <flags.hpp>

int main(int argc, char* argv[]){
    if(argc == 1){
        print_line("Usage: touch file_path");
        exit(1);
    }

    auto fd = open(argv[1], std::OPEN_CREATE);

    if(fd.valid()){
        close(*fd);
    } else {
        printf("touch: error: %s\n", std::error_message(fd.error()));
    }

    exit(0);
}