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

std::string read_file(const std::string& path, bool check = true){
    tlib::file f(path);

    if(!f){
        if(check){
            tlib::printf("ifconfig: error: %s\n", std::error_message(f.error()));
        }

        return "";
    }

    auto contents = f.read_file();

    if(!f){
        tlib::printf("ifconfig: error: %s\n", std::error_message(f.error()));
    }

    return contents;
}

int main(int /*argc*/, char* /*argv*/[]){
    auto str_virt_memory_avail = read_file("/sys/memory/virtual/available");
    auto str_virt_memory_free = read_file("/sys/memory/virtual/free");
    auto str_virt_memory_alloc = read_file("/sys/memory/virtual/allocated");

    if(str_virt_memory_avail.empty() || str_virt_memory_free.empty() || str_virt_memory_alloc.empty()){
        return 1;
    }

    auto str_phys_memory_avail = read_file("/sys/memory/physical/available");
    auto str_phys_memory_free = read_file("/sys/memory/physical/free");
    auto str_phys_memory_alloc = read_file("/sys/memory/physical/allocated");

    if(str_phys_memory_avail.empty() || str_phys_memory_free.empty() || str_phys_memory_alloc.empty()){
        return 1;
    }

    auto virt_memory_avail = atoui(str_virt_memory_avail);
    auto virt_memory_free = atoui(str_virt_memory_free);
    auto virt_memory_alloc = atoui(str_virt_memory_alloc);

    auto phys_memory_avail = atoui(str_phys_memory_avail);
    auto phys_memory_free = atoui(str_phys_memory_free);
    auto phys_memory_alloc = atoui(str_phys_memory_alloc);

    tlib::printf(" Virtual: available:%m free:%m allocated:%m\n", virt_memory_avail, virt_memory_free, virt_memory_alloc);

    if(virt_memory_avail - (virt_memory_alloc + virt_memory_free)){
        tlib::printf("         diffrence!::%m\n", virt_memory_avail - (virt_memory_alloc + virt_memory_free));
    }

    tlib::printf("Physical: available:%m free:%m allocated:%m\n", phys_memory_avail, phys_memory_free, phys_memory_alloc);

    if(phys_memory_avail - (phys_memory_alloc + phys_memory_free)){
        tlib::printf("         diffrence!::%m\n", phys_memory_avail - (phys_memory_alloc + phys_memory_free));
    }

    return 0;
}
