//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>
#include <errors.hpp>

#include "fs/procfs.hpp"

#include "scheduler.hpp"

namespace {

const scheduler::process_control_t* pcb = nullptr;

} //end of anonymous namespace

void procfs::set_pcb(const scheduler::process_control_t* pcb_ptr){
    pcb = pcb_ptr;
}

procfs::procfs_file_system::procfs_file_system(std::string mp) : mount_point(mp) {
    //Nothing to init
}

procfs::procfs_file_system::~procfs_file_system(){
    //Nothing to delete
}

size_t procfs::procfs_file_system::get_file(const std::vector<std::string>& file_path, vfs::file& f){
    if(file_path.empty()){
        f.file_name = "/";
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    if(file_path.size() == 1){
        //TODO
    }

    return std::ERROR_NOT_EXISTS;
}

size_t procfs::procfs_file_system::read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    //Cannot access the root for reading
    if(file_path.size() < 2){
        return std::ERROR_PERMISSION_DENIED;
    }

    return std::ERROR_NOT_EXISTS;
}

size_t procfs::procfs_file_system::ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents){
    if(file_path.size() == 0){
        for(size_t i = 0; i < scheduler::MAX_PROCESS; ++i){
            auto& process = pcb[i];

            if(process.state != scheduler::process_state::EMPTY){
                vfs::file f;
                f.file_name = std::to_string(process.process.pid);
                f.directory = false;
                f.hidden = false;
                f.system = false;
                f.size = 0;
                contents.emplace_back(std::move(f));
            }
        }
    }

    //No subfolder support
    if(file_path.size() > 0){
        return std::ERROR_NOT_EXISTS;
    }

    return 0;
}

size_t procfs::procfs_file_system::statfs(statfs_info& file){
    file.total_size = 0;
    file.free_size = 0;

    return 0;
}

size_t procfs::procfs_file_system::write(const std::vector<std::string>& file_path, const char* buffer, size_t count, size_t offset, size_t& written){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::truncate(const std::vector<std::string>&, size_t){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::touch(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::mkdir(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}

size_t procfs::procfs_file_system::rm(const std::vector<std::string>& ){
    return std::ERROR_PERMISSION_DENIED;
}
