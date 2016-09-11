//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "assert.hpp"
#include "console.hpp"
#include "virtual_debug.hpp"
#include "early_memory.hpp"
#include "vfs/vfs.hpp"

#include <flags.hpp>
#include <string.hpp>

namespace {

bool early_mode = true;
bool file = false;

std::string buffer;

inline const char* level_to_string(logging::log_level level){
    switch(level){
        case logging::log_level::TRACE:
            return "TRACE";
        case logging::log_level::DEBUG:
            return "DEBUG";
        case logging::log_level::WARNING:
            return "WARNING";
        case logging::log_level::ERROR:
            return "ERROR";
    }

    return "UNKNOWN";
}

void append_to_file(const char* s, size_t length){
    auto fd = vfs::open("/messages", std::OPEN_CREATE);

    if(fd >= 0){
        stat_info info;
        if(vfs::stat(fd, info) == 0){
            if(vfs::truncate(fd, info.size + length + 1) == 0){
                std::string buffer = s;
                buffer += '\n';

                vfs::write(fd, buffer.c_str(), buffer.size(), info.size);
            }
        }

        vfs::close(fd);
    }
}

} //end of anonymous namespace

bool logging::is_early(){
    return early_mode;
}

bool logging::is_file(){
    return file;
}

void logging::finalize(){
    //Starting from there, the messages will be sent to the terminal
    early_mode = false;

    logf(log_level::TRACE, "%u early logs \n", early::early_logs_count());

    auto table_address = early::early_logs_address;

    for(size_t i = 0; i < early::early_logs_count(); ++i){
        auto string_address = size_t(*reinterpret_cast<uint32_t*>(table_address + i * 4));

        // Print to the virtual debugger
        virtual_debug("EARLY: ");
        virtual_debug(reinterpret_cast<const char*>(string_address));
        virtual_debug("\n");
    }
}

void logging::to_file(){
    thor_assert(false, "logging to file needs revision");

    //Starting from there, the messages will be sent to the log file
    file = true;
}

void logging::log(log_level level, const char* s){
    if(!is_early()){
        // Print to the virtual debugger
        virtual_debug(level_to_string(level));
        virtual_debug(": ");
        virtual_debug(s);
    }

    if(is_file()){
        append_to_file(s, std::str_len(s));
    }
}

void logging::log(log_level level, const std::string& s){
    thor_assert(!is_early(), "log(level,string) is not valid in early mode");

    log(level, s.c_str());
}

void logging::logf(log_level level, const char* s, va_list va){
    char buffer[1024];
    vsprintf_raw(buffer, 1024, s, va);
    log(level, buffer);
}

void logging::logf(log_level level, const char* s, ...){
    va_list va;
    va_start(va, s);

    char buffer[1024];
    vsprintf_raw(buffer, 1024, s, va);
    log(level, buffer);

    va_end(va);
}
