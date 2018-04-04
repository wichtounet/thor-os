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
#include <tlib/directory_entry.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

std::string read_file(const std::string& path){
    auto fd = tlib::open(path.c_str());

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            auto size = info->size;

            auto buffer = new char[size+1];

            auto content_result = tlib::read(*fd, buffer, size);

            if(content_result.valid()){
                if(*content_result != size){
                    //TODO Read more
                } else {
                    buffer[size] = '\0';
                    tlib::close(*fd);
                    return buffer;
                }
            } else {
                tlib::printf("cat: error: %s\n", std::error_message(content_result.error()));
            }
        } else {
            tlib::printf("cat: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("cat: error: %s\n", std::error_message(fd.error()));
    }

    return "";
}

int main(int /*argc*/, char* /*argv*/[]){
    auto fd = tlib::open("/sys/memory/e820/");

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            auto entries_buffer = new char[BUFFER_SIZE];

            auto entries_result = tlib::entries(*fd, entries_buffer, BUFFER_SIZE);

            if(entries_result.valid()){
                size_t position = 0;

                while(true){
                    auto entry = reinterpret_cast<tlib::directory_entry*>(entries_buffer + position);

                    std::string base_path = "/sys/memory/e820/";
                    std::string entry_name = &entry->name;

                    if(entry_name != "entries"){
                        auto base = parse(read_file(base_path + entry_name + "/base"));
                        auto size = parse(read_file(base_path + entry_name + "/size"));
                        auto type = read_file(base_path + entry_name + "/type");

                        tlib::printf("%s: %s (%hB) %h -> %h\n", &entry->name, type.c_str(), size, base, base + size);
                    }

                    if(!entry->offset_next){
                        break;
                    }

                    position += entry->offset_next;
                }
            } else {
                tlib::printf("lse820: entries error: %s\n", std::error_message(entries_result.error()));
            }

            delete[] entries_buffer;
        } else {
            tlib::printf("lse820: stat error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("lse820: open error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
