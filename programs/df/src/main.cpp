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
#include <tlib/statfs_info.hpp>

static constexpr const size_t BUFFER_SIZE = 4096;

int main(int argc, char* argv[]){
    bool human = false;

    for(size_t i = 1; i < size_t(argc); ++i){
        std::string param(argv[i]);

        if(param == "-h"){
            human = true;
        }
    }

    auto buffer = new char[BUFFER_SIZE];

    auto mp_result = tlib::mounts(buffer, BUFFER_SIZE);

    if(mp_result.valid()){
        size_t position = 0;

        tlib::print_line("File system Size Used Available");

        while(true){
            auto entry = reinterpret_cast<tlib::mount_point*>(buffer + position);

            auto mount_point = &entry->name;

            auto statfs_result = tlib::statfs(mount_point);

            if(statfs_result.valid()){
                auto& statfs = *statfs_result;

                if(human){
                    tlib::printf("%s %m %m %m\n", mount_point, statfs.total_size, statfs.total_size - statfs.free_size, statfs.free_size);
                } else {
                    tlib::printf("%s %u %u %u\n", mount_point, statfs.total_size, statfs.total_size - statfs.free_size, statfs.free_size);
                }
            } else {
                tlib::printf("df: error: %s\n", std::error_message(statfs_result.error()));
            }

            if(!entry->offset_next){
                break;
            }

            position += entry->offset_next;
        }
    } else {
        tlib::printf("df: error: %s\n", std::error_message(mp_result.error()));
    }

    delete[] buffer;

    return 0;
}
