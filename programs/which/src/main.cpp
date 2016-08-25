//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <tlib/file.hpp>
#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>

int main(int argc, char* argv[]){
    if(argc == 1){
        tlib::print_line("Usage: which executable_path");
        return 1;
    }

    std::string path(argv[1]);

    if(path[0] != '/'){
        path = "/bin/" + path;
    }

    auto fd = tlib::open(path.c_str());

    if(fd.valid()){
        tlib::print_line(path);

        tlib::close(*fd);
    } else {
        if(fd.has_error(std::ERROR_NOT_EXISTS)){
            tlib::printf("%s not found\n", argv[1]);
        } else {
            tlib::printf("which: error: %s\n", std::error_message(fd.error()));
        }
    }

    return 0;
}
