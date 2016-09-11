#include <types.hpp>
#include <unique_ptr.hpp>
#include <algorithms.hpp>
#include <errors.hpp>

#include "fs/procfs.hpp"

#include "scheduler.hpp"
#include "logging.hpp"

namespace {

const scheduler::process_control_t* pcb = nullptr;

std::vector<vfs::file> standard_contents;

uint64_t atoui(const std::string& s){
    uint64_t value = 0;
    uint64_t mul = 1;

    for(size_t i = s.size(); i > 0; --i){
        auto c = s[i - 1];

        if(c < '0' || c  > '9'){
            return value;
        }

        value += mul * (c - '0');

        mul *= 10;
    }

    return value;
}

size_t read(const std::string& value, char* buffer, size_t count, size_t offset, size_t& read){
    if(offset > value.size()){
        return std::ERROR_INVALID_OFFSET;
    }

    read = std::min(count, value.size() - offset);
    std::copy_n(buffer, value.c_str() + offset, read);

    return 0;
}

std::string get_value(uint64_t pid, const std::string& name){
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
    } else {
        return "";
    }
}

} //end of anonymous namespace

void procfs::set_pcb(const scheduler::process_control_t* pcb_ptr){
    pcb = pcb_ptr;
}

procfs::procfs_file_system::procfs_file_system(std::string mp) : mount_point(mp) {
    standard_contents.emplace_back("pid", false, false, false, 0);
    standard_contents.emplace_back("ppid", false, false, false, 0);
    standard_contents.emplace_back("state", false, false, false, 0);
    standard_contents.emplace_back("system", false, false, false, 0);
    standard_contents.emplace_back("priority", false, false, false, 0);
}

procfs::procfs_file_system::~procfs_file_system(){
    //Nothing to delete
}

size_t procfs::procfs_file_system::get_file(const std::vector<std::string>& file_path, vfs::file& f){
    // Access the root folder
    if(file_path.empty()){
        f.file_name = "/";
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    auto i = atoui(file_path[0]);

    // Check the pid folder
    if(i >= scheduler::MAX_PROCESS){
        return std::ERROR_NOT_EXISTS;
    }

    auto& process = pcb[i];

    if(process.state == scheduler::process_state::EMPTY){
        return std::ERROR_NOT_EXISTS;
    }

    // Access a pid folder
    if(file_path.size() == 1){
        f.file_name = file_path[0];
        f.directory = true;
        f.hidden = false;
        f.system = false;
        f.size = 0;

        return 0;
    }

    // Access a file directly
    if(file_path.size() == 2){
        auto value = get_value(i, file_path[1]);

        if(value.size()){
            f.file_name = file_path[1];
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

size_t procfs::procfs_file_system::read(const std::vector<std::string>& file_path, char* buffer, size_t count, size_t offset, size_t& read){
    //Cannot access the root nor the pid directores for reading
    if(file_path.size() < 2){
        return std::ERROR_PERMISSION_DENIED;
    }

    if(file_path.size() == 2){
        auto i = atoui(file_path[0]);

        if(i >= scheduler::MAX_PROCESS){
            return std::ERROR_NOT_EXISTS;
        }

        auto& process = pcb[i];

        if(process.state == scheduler::process_state::EMPTY){
            return std::ERROR_NOT_EXISTS;
        }

        auto value = get_value(i, file_path[1]);

        if(value.size()){
            return ::read(value, buffer, count, offset, read);
        } else {
            return std::ERROR_NOT_EXISTS;
        }
    }

    return std::ERROR_NOT_EXISTS;
}

size_t procfs::procfs_file_system::ls(const std::vector<std::string>& file_path, std::vector<vfs::file>& contents){
    logging::logf(logging::log_level::DEBUG, "procfs ls %u\n", file_path.size());

    if(file_path.size() == 0){
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

    if(file_path.size() == 1){
        contents = standard_contents;
        return 0;
    }

    //No more subfolder support
    return std::ERROR_NOT_EXISTS;
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
