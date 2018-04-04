//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/file.hpp>
#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/flags.hpp>

int main(int argc, char* argv[]){
    if(argc == 1){
        tlib::print_line("Usage: rm file_path");
        return 1;
    }

    auto fd = tlib::open(argv[1]);

    if(fd.valid()){
        auto result = tlib::rm(argv[1]);

        if(result < 0){
            tlib::printf("rm: error: %s\n", std::error_message(-result));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("rm: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
