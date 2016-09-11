//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <file.hpp>
#include <system.hpp>
#include <errors.hpp>
#include <print.hpp>
#include <directory_entry.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

std::string read_file(const std::string& path){
    auto fd = open(path.c_str());

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            auto size = info->size;

            auto buffer = new char[size+1];

            auto content_result = read(*fd, buffer, size);

            if(content_result.valid()){
                if(*content_result != size){
                    //TODO Read more
                } else {
                    buffer[size] = '\0';
                    close(*fd);
                    return buffer;
                }
            } else {
                printf("cat: error: %s\n", std::error_message(content_result.error()));
            }
        } else {
            printf("cat: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("cat: error: %s\n", std::error_message(fd.error()));
    }

    return "";
}

int main(int /*argc*/, char* /*argv*/[]){
    auto fd = open("/sys/memory/e820/");

    if(fd.valid()){
        auto info = stat(*fd);

        if(info.valid()){
            auto entries_buffer = new char[BUFFER_SIZE];

            auto entries_result = entries(*fd, entries_buffer, BUFFER_SIZE);

            if(entries_result.valid()){
                size_t position = 0;

                while(true){
                    auto entry = reinterpret_cast<directory_entry*>(entries_buffer + position);

                    std::string base_path = "/sys/memory/e820/";
                    std::string entry_name = &entry->name;

                    if(entry_name != "entries"){
                        auto base = parse(read_file(base_path + entry_name + "/base"));
                        auto size = parse(read_file(base_path + entry_name + "/size"));
                        auto type = read_file(base_path + entry_name + "/type");

                        printf("%s: %s (%hB) %h -> %h\n", &entry->name, type.c_str(), size, base, base + size);
                    }

                    if(!entry->offset_next){
                        break;
                    }

                    position += entry->offset_next;
                }
            } else {
                printf("lse820: error: %s\n", std::error_message(entries_result.error()));
            }

            delete[] entries_buffer;
        } else {
            printf("lse820: error: %s\n", std::error_message(info.error()));
        }

        close(*fd);
    } else {
        printf("lse820: error: %s\n", std::error_message(fd.error()));
    }

    exit(0);
}
