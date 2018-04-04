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
        tlib::print_line("Usage: writer file_path");
        return 1;
    }

    auto fd = tlib::open(argv[1], std::OPEN_CREATE);

    if(fd.valid()){
        auto truncate_result = tlib::truncate(*fd, 12);

        if(truncate_result.valid()){
            auto s = "0123456789AB";

            auto write_result = tlib::write(*fd, s, 12, 0);

            if(write_result.valid()){
                //TODO
            } else {
                tlib::printf("writer: error: %s\n", std::error_message(write_result.error()));
            }
        } else {
            tlib::printf("writer: error: %s\n", std::error_message(truncate_result.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("writer: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
