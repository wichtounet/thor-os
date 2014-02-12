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

int main(){
    auto fd = open("/stat");

    if(fd.valid()){
        printf("fd: %u\n", *fd);
    } else {
        printf("stat: error: %s\n", std::error_message(fd.error()));
    }

    exit(0);
}