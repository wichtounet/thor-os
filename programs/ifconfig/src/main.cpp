//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
                tlib::printf("ifconfig: error: %s\n", std::error_message(content_result.error()));
            }
        } else {
            tlib::printf("ifconfig: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("ifconfig: error: %s\n", std::error_message(fd.error()));
    }

    return "";
}

int main(int /*argc*/, char* /*argv*/[]){
    auto fd = tlib::open("/sys/net/");

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            auto entries_buffer = new char[BUFFER_SIZE];

            auto entries_result = tlib::entries(*fd, entries_buffer, BUFFER_SIZE);

            if(entries_result.valid()){
                size_t position = 0;

                while(true){
                    auto entry = reinterpret_cast<tlib::directory_entry*>(entries_buffer + position);

                    std::string base_path = "/sys/net/";
                    std::string entry_name = &entry->name;

                    auto enabled = read_file(base_path + entry_name + "/enabled");

                    if(enabled == "true"){
                        auto driver = read_file(base_path + entry_name + "/driver");
                        auto ip = read_file(base_path + entry_name + "/ip");
                        auto mac = read_file(base_path + entry_name + "/mac");

                        tlib::printf("%10s inet %s\n", &entry->name, ip.c_str());

                        if(!mac.empty() && mac != "0"){
                            tlib::printf("%10s ether %s\n", "", mac.c_str());
                        }

                        tlib::printf("%10s driver %s\n", "", driver.c_str());
                    }

                    if(!entry->offset_next){
                        break;
                    }

                    tlib::printf("\n");

                    position += entry->offset_next;
                }
            } else {
                tlib::printf("ifconfig: error: %s\n", std::error_message(entries_result.error()));
            }

            delete[] entries_buffer;
        } else {
            tlib::printf("ifconfig: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("ifconfig: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
