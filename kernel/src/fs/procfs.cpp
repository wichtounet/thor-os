//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>
#include <string.hpp>

#include <tlib/errors.hpp>

#include "fs/procfs.hpp"

#include "scheduler.hpp"
#include "logging.hpp"

namespace {

const scheduler::process_control_t* pcb = nullptr;

std::vector<vfs::file> standard_contents;

size_t read(const std::string& value, char* buffer, size_t count, size_t offset, size_t& read){
    if(offset > value.size()){
        return std::ERROR_INVALID_OFFSET;
    }

    read = std::min(count, value.size() - offset);
    std::copy_n(value.c_str() + offset, read, buffer);

    return 0;
}

std::string get_value(uint64_t pid, std::string_view name){
    auto& process = pcb[pid];

    if(name == "pid"){
        return std::to_string(process.process.pid);
    } else if(name == "ppid"){
        return std::to_string(process.process.ppid);
    } else if(name == "state"){
        return std::to_string(static_cast<uint8_t>(process.state));
    } else if(name == "system"){
        return process.process.system ? "true" : "false";
    } else if(name == "priority"){
        return std::to_string(process.process.priority);
    } else if(name == "name"){
        return process.process.name;
    } else if(name == "memory"){
        return std::to_string(process.process.brk_end - process.process.brk_start);
    } else {
        return "";
    }
}

} //end of anonymous namespace

void procfs::set_pcb(const scheduler::process_control_t* pcb_ptr){
    pcb = pcb_ptr;
}

procfs::procfs_file_system::procfs_file_system(path mp) : mount_point(mp) {
    standard_contents.reserve(7);
    standard_contents.emplace_back("pid", false, false, false, 0UL);
    standard_contents.emplace_back("ppid", false, false, false, 0UL);
    standard_contents.emplace_back("state", false, false, false, 0UL);
    standard_contents.emplace_back("system", false, false, false, 0UL);
    standard_contents.emplace_back("priority", false, false, false, 0UL);
    standard_contents.emplace_back("name", false, false, false, 0UL);
    standard_contents.emplace_back("memory", false, false, false, 0UL);
}

procfs::procfs_file_system::~procfs_file_system(){
    //Nothing to delete
}

size_t procfs::procfs_file_system::get_file(const path& file_path, vfs::file& f){
    // Access the root folder
    if(file_path.is_root()){
        f.file_name = "/";
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    auto i = atoui(file_path[1]);

    // Check the pid folder
    if(i >= scheduler::MAX_PROCESS){
        return std::ERROR_NOT_EXISTS;
    }

    auto& process = pcb[i];

    if(process.state == scheduler::process_state::EMPTY){
        return std::ERROR_NOT_EXISTS;
    }

    // Access a pid folder
    if(file_path.size() == 2){
        f.file_name = file_path[1];
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    // Access a file directly
    if(file_path.size() == 3){
        auto value = get_value(i, file_path[2]);

        if(value.size()){
            f.file_name = file_path[2];
            f.directory = false;
            f.hidden = false;
            f.system = false;
            f.size = value.size();

            return 0;
        } else {
            return std::ERROR_NOT_EXISTS;
        }
    }

    // There are no more levels
    return std::ERROR_NOT_EXISTS;
}

size_t procfs::procfs_file_system::read(const path& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    //Cannot access the root nor the pid directores for reading
    if(file_path.size() < 3){
        return std::ERROR_PERMISSION_DENIED;
    }

    if(file_path.size() == 3){
        auto i = atoui(file_path[1]);

        if(i >= scheduler::MAX_PROCESS){
            return std::ERROR_NOT_EXISTS;
        }

        auto& process = pcb[i];

        if(process.state == scheduler::process_state::EMPTY){
            return std::ERROR_NOT_EXISTS;
        }

        auto value = get_value(i, file_path[2]);

        if(value.size()){
            return ::read(value, buffer, count, offset, read);
        } else {
            return std::ERROR_NOT_EXISTS;
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t procfs::procfs_file_system::read(const path& /*file_path*/, char* /*buffer*/, size_t /*count*/, size_t /*offset*/, size_t& /*read*/, size_t /*ms*/){
    return std::ERROR_UNSUPPORTED;
}

size_t procfs::procfs_file_system::ls(const path& file_path, std::vector<vfs::file>& contents){
    if(file_path.is_root()){
        for(size_t i = 0; i < scheduler::MAX_PROCESS; ++i){
            auto& process = pcb[i];

            if(process.state != scheduler::process_state::EMPTY){
                vfs::file f;
                f.file_name = std::to_string(process.process.pid);
                f.directory = true;
                f.hidden = false;
                f.system = false;
                f.size = 0;
                contents.emplace_back(std::move(f));
            }
        }

        return 0;
    }

    if(file_path.size() == 2){
        contents = standard_contents;
        return 0;
    }

    //No more subfolder support
    return std::ERROR_NOT_EXISTS;
}

size_t procfs::procfs_file_system::statfs(vfs::statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

size_t procfs::procfs_file_system::write(const path&, const char*, size_t, size_t, size_t&){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::clear(const path&, size_t, size_t, size_t&){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::truncate(const path&, size_t){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::touch(const path& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::mkdir(const path& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::rm(const path& ){
    return std::ERROR_PERMISSION_DENIED;
}
