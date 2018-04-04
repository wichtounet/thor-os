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
        tlib::print_line("Usage: mkdir file_path");
        return 1;
    }

    auto result = tlib::mkdir(argv[1]);

    if(result < 0){
        tlib::printf("mkdir: error: %s\n", std::error_message(-result));
    }

    return 0;
}
