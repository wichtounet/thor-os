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
        print_line("Usage: stat file_path");
        return 1;
    }

    auto fd = open(argv[1]);

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            if(info->flags & STAT_FLAG_DIRECTORY){
                print("Directory ");
            } else {
                print("File ");
            }

            print_line(argv[1]);

            printf("Size: %m\n", info->size);
            print("Flags: ");

            if(info->flags & STAT_FLAG_HIDDEN){
                print("Hidden ");
            }

            if(info->flags & STAT_FLAG_SYSTEM){
                print("System ");
            }

            print_line();

            print("Created: ");

            print(info->created.day);
            print('.');
            print(info->created.month);
            print('.');
            print(info->created.year);
            print(' ');

            print(info->created.hour);
            print(':');
            print(info->created.minutes);
            print_line();

            print("Modified: ");

            print(info->modified.day);
            print('.');
            print(info->modified.month);
            print('.');
            print(info->modified.year);
            print(' ');

            print(info->modified.hour);
            print(':');
            print(info->modified.minutes);
            print_line();

            print("Accessed: ");

            print(info->accessed.day);
            print('.');
            print(info->accessed.month);
            print('.');
            print(info->accessed.year);
            print(' ');

            print(info->accessed.hour);
            print(':');
            print(info->accessed.minutes);
            print_line();
        } else {
            printf("stat: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("stat: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
