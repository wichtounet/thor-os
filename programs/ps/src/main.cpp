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

namespace {

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
                tlib::printf("ps: error: %s\n", std::error_message(content_result.error()));
            }
        } else {
            tlib::printf("ps: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("ps: error: %s\n", std::error_message(fd.error()));
    }

    return "";
}

const char* state_str(uint64_t state){
    switch(state){
        case 0:
            return "EMPTY";
        case 1:
            return "NEW";
        case 2:
            return "READY";
        case 3:
            return "RUNNING";
        case 4:
            return "BLOCKED";
        case 5:
            return "SLEEPING";
        case 6:
            return "WAITING";
        case 7:
            return "KILLED";
        default:
            return "UNKNOWN";
    }
}

} // end of anonymous space

int main(int /*argc*/, char* /*argv*/[]){
    auto fd = tlib::open("/proc/");

    tlib::printf("PID PPID Pri State      Memory Name\n");

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            auto entries_buffer = new char[BUFFER_SIZE];

            auto entries_result = tlib::entries(*fd, entries_buffer, BUFFER_SIZE);

            if(entries_result.valid()){
                size_t position = 0;

                while(true){
                    auto entry = reinterpret_cast<tlib::directory_entry*>(entries_buffer + position);

                    std::string base_path = "/proc/";
                    std::string entry_name = &entry->name;

                    auto pid = parse(read_file(base_path + entry_name + "/pid"));
                    auto ppid = parse(read_file(base_path + entry_name + "/ppid"));
                    auto system = read_file(base_path + entry_name + "/system") == "true";
                    auto priority = parse(read_file(base_path + entry_name + "/priority"));
                    auto state = parse(read_file(base_path + entry_name + "/state"));
                    auto name = read_file(base_path + entry_name + "/name");
                    auto memory = parse(read_file(base_path + entry_name + "/memory"));

                    if(system){
                        tlib::printf("%3u %4u %3u %10s %6m %s [kernel]\n", pid, ppid, priority, state_str(state), memory, name.c_str());
                    } else {
                        tlib::printf("%3u %4u %3u %10s %6m %s \n", pid, ppid, priority, state_str(state), memory, name.c_str());
                    }

                    if(!entry->offset_next){
                        break;
                    }

                    position += entry->offset_next;
                }
            } else {
                tlib::printf("ps: error: %s\n", std::error_message(entries_result.error()));
            }

            delete[] entries_buffer;
        } else {
            tlib::printf("ps: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("ps: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
