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
#include <tlib/directory_entry.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

namespace {

struct config {
    bool list = false;
    bool hidden = false;
};

void ls_files(const config& conf, const char* file_path){
    auto fd = open(file_path);

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            if(!(info->flags & STAT_FLAG_DIRECTORY)){
                print_line("ls: error: Is not a directory");
            } else {
                auto buffer = new char[BUFFER_SIZE];

                auto entries_result = entries(*fd, buffer, BUFFER_SIZE);

                if(entries_result.valid()){
                    if(*entries_result){
                        size_t position = 0;

                        while(true){
                            auto entry = reinterpret_cast<directory_entry*>(buffer + position);

                            bool show = true;
                            if(!conf.hidden){
                                auto path = std::string(file_path) + "/" + &entry->name;

                                auto file_fd = open(path.c_str());

                                if(file_fd.valid()){
                                    auto file_info = stat(*file_fd);

                                    if(file_info.valid()){
                                        if(file_info->flags & STAT_FLAG_HIDDEN){
                                            show = false;
                                        }
                                    } else {
                                        printf("ls: stat error: %s\n", std::error_message(file_info.error()));
                                    }
                                } else {
                                    printf("ls: open error: %s\n", std::error_message(file_fd.error()));
                                }

                                close(*file_fd);
                            }

                            if(show){
                                if(conf.list){
                                    print_line(&entry->name);
                                } else {
                                    print(&entry->name);
                                    print(" ");
                                }
                            }

                            if(!entry->offset_next){
                                break;
                            }

                            position += entry->offset_next;
                        }

                        if(!conf.list){
                            print_line();
                        }
                    }
                } else {
                    printf("ls: entries error: %s\n", std::error_message(entries_result.error()));
                }

                delete[] buffer;
            }
        } else {
            printf("ls: stat error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("ls: open error: %s\n", std::error_message(fd.error()));
    }
}

} // end of anonymous namespace

int main(int argc, char* argv[]){
    config conf;

    int i = 1;
    for(; i < argc; ++i){
        std::string arg(argv[i]);

        if(arg[0] == '-'){
            for(size_t j = 1; j < arg.size(); ++j){
                auto c = arg[j];

                if(c == 'l'){
                    conf.list = true;
                } else if(c == 'a'){
                    conf.hidden = true;
                } else {
                    print_line("ls: invalid argument");
                    return 1;
                }
            }
        } else {
            break;
        }
    }

    if(i == argc){
        auto cwd = current_working_directory();
        ls_files(conf, cwd.c_str());
    } else {
        ls_files(conf, argv[argc -1]);
    }

    return 0;
}
