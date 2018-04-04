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
#include <tlib/mount_point.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

int main(int argc, char* argv[]){
    if(argc == 1){
        auto buffer = new char[BUFFER_SIZE];

        auto mp_result = tlib::mounts(buffer, BUFFER_SIZE);

        if(mp_result.valid()){
            size_t position = 0;

            while(true){
                auto entry = reinterpret_cast<tlib::mount_point*>(buffer + position);

                auto mount_point = &entry->name;
                auto device = mount_point + entry->length_mp + 1;
                auto type = device + entry->length_dev + 1;

                tlib::printf("%s %s %s\n", mount_point, device, type);

                if(!entry->offset_next){
                    break;
                }

                position += entry->offset_next;
            }
        } else {
            tlib::printf("mount: error: %s\n", std::error_message(mp_result.error()));
        }

        delete[] buffer;

        return 0;
    }

    if(argc < 4){
        tlib::printf("usage: mount fs device mountpoint\n");

        return 1;
    }

    auto fs_str = argv[1];
    auto device_str = argv[2];
    auto mount_point_str = argv[3];

    std::string fs(fs_str);

    if(fs == "fat32"){
        tlib::printf("mkfs: Mounting %s fat32 filesystem on %s\n", device_str, mount_point_str);

        auto device_fd = tlib::open(device_str);

        if(!device_fd.valid()){
            tlib::printf("mount: open error: %s\n", std::error_message(device_fd.error()));
            return 1;
        }

        auto mp_fd = tlib::open(mount_point_str);

        if(!mp_fd.valid()){ tlib::printf("mount: open error: %s\n", std::error_message(mp_fd.error()));
            return 1;
        }

        auto mount_result = tlib::mount(1, *device_fd, *mp_fd);

        if(!mount_result.valid()){
            tlib::printf("mount: mount error: %s\n", std::error_message(mount_result.error()));
            return 1;
        }
    } else {
        tlib::printf("mkfs: Unsupported filesystem %s\n", fs_str);
        return 1;
    }

    return 0;
}
