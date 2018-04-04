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

const char* device_type_str(const std::string& class_code){
    if(class_code == "1"){
        return "Mass Storage Controller";
    } else if(class_code == "2"){
        return "Network Controller";
    } else if(class_code == "3"){
        return "Display Controller";
    } else if(class_code == "4"){
        return "Multimedia Controller";
    } else if(class_code == "5"){
        return "Memory Controller";
    } else if(class_code == "6"){
        return "Bridge Device";
    } else if(class_code == "7"){
        return "Simple Communications Controller";
    } else if(class_code == "8"){
        return "Base System Peripheral";
    } else if(class_code == "9"){
        return "Input Device";
    } else if(class_code == "10"){
        return "Docking Station";
    } else if(class_code == "11"){
        return "Processors";
    } else if(class_code == "12"){
        return "Serial Bus Controller";
    } else if(class_code == "13"){
        return "Wireless Controller";
    } else if(class_code == "14"){
        return "Intelligent I/O Controller";
    } else if(class_code == "15"){
        return "Satellite Communication Controller";
    } else if(class_code == "16"){
        return "Encryption/Decryption Controller";
    } else if(class_code == "17"){
        return "Data Acquisition and Signal Processing Controller";
    } else {
        return "Unknown Device";
    }
}

int main(int /*argc*/, char* /*argv*/[]){
    auto fd = tlib::open("/sys/pci/");

    if(fd.valid()){
        auto info = tlib::stat(*fd);

        if(info.valid()){
            auto entries_buffer = new char[BUFFER_SIZE];

            auto entries_result = tlib::entries(*fd, entries_buffer, BUFFER_SIZE);

            if(entries_result.valid()){
                size_t position = 0;

                while(true){
                    auto entry = reinterpret_cast<tlib::directory_entry*>(entries_buffer + position);

                    std::string base_path = "/sys/pci/";
                    std::string entry_name = &entry->name;

                    auto device = read_file(base_path + entry_name + "/device");
                    auto vendor = read_file(base_path + entry_name + "/vendor");
                    auto class_code = read_file(base_path + entry_name + "/class");
                    auto subclass = read_file(base_path + entry_name + "/subclass");
                    auto device_type = device_type_str(class_code);

                    tlib::printf("%s: %s %s:%s (%s:%s)\n", device_type, &entry->name, vendor.c_str(), device.c_str(), class_code.c_str(), subclass.c_str());

                    if(!entry->offset_next){
                        break;
                    }

                    position += entry->offset_next;
                }
            } else {
                tlib::printf("ls: error: %s\n", std::error_message(entries_result.error()));
            }

            delete[] entries_buffer;
        } else {
            tlib::printf("ls: error: %s\n", std::error_message(info.error()));
        }

        tlib::close(*fd);
    } else {
        tlib::printf("ls: error: %s\n", std::error_message(fd.error()));
    }

    return 0;
}
