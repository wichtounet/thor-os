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
#include <flags.hpp>

int main(int argc, char* argv[]){
    if(argc == 1){
        print_line("Usage: mkdir file_path");
        exit(1);
    }

    auto result = mkdir(argv[1]);

    if(result < 0){
        printf("mkdir: error: %s\n", std::error_message(-result));
    }

    exit(0);
}