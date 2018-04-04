//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/file.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/flags.hpp>

int main(int argc, char* argv[]){
    if(argc == 1){
        tlib::print_line("Usage: touch file_path");
        return 1;
    }

    auto fd = tlib::open(argv[1], std::OPEN_CREATE);

    if(fd.valid()){
        tlib::close(*fd);
    } else {
        tlib::printf("touch: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
