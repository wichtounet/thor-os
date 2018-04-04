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

int main(int argc, char* argv[]){
    if(argc == 1){
        tlib::print_line("Usage: cat file_path");
        return 1;
    }

    auto fd = tlib::open(argv[1]);

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            if(info->flags & tlib::STAT_FLAG_DIRECTORY){
                tlib::print_line("cat: error: Is a directory");
            } else {
                auto size = info->size;

                auto buffer = new char[size];

                auto content_result = tlib::read(*fd, buffer, size);

                if(content_result.valid()){
                    if(*content_result != size){
                        //TODO Read more
                    } else {
                        for(size_t i = 0; i < size; ++i){
                            tlib::print(buffer[i]);
                        }

                        tlib::print_line();
                    }
                } else {
                    tlib::printf("cat: error: %s\n", std::error_message(content_result.error()));
                }
            }
        } else {
            tlib::printf("cat: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("cat: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
