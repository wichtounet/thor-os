//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "early_logging.hpp"
#include "assert.hpp"
#include "console.hpp"
#include "virtual_debug.hpp"
#include "vfs/vfs.hpp"

#include <flags.hpp>
#include <string.hpp>

namespace {

bool early_mode = true;
bool file = false;

constexpr const size_t MAX_EARLY = 128;

size_t current_early = 0;
const char* early_logs[MAX_EARLY];

const char* new_line = "\n";

std::string buffer;

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
}

void logging::to_file(){
    //Starting from there, the messages will be sent to the log file
    file = true;

    for(size_t i = 0; i < early::early_logs_count; ++i){
        auto early_log = early::early_logs[i];

        auto early_log_str = reinterpret_cast<const char*>(static_cast<size_t>(early_log));

        append_to_file(early_log_str, std::str_len(early_log_str));
    }
}

void logging::log(const char* s){
    if(!is_early()){
        //TODO Display the string in the kernel terminal
    }

    if(is_file()){
        append_to_file(s, std::str_len(s));
    } else {
        thor_assert(current_early < MAX_EARLY, "early log buffer is full");

        early_logs[current_early++] = s;
    }
}

void logging::logf(const char* format, ...){
    thor_assert(!is_early(), "log(string) in only valid in normal mode");
    thor_assert(is_file(), "log(string) in only valid file mode");

    //TODO Display the string in the kernel terminal

    va_list va;
    va_start(va, format);

    auto s = vsprintf(format, va);

    va_end(va);

    append_to_file(s.c_str(), s.size());
}

void logging::log(const std::string& s){
    thor_assert(!is_early(), "log(string) in only valid in normal mode");
    thor_assert(is_file(), "log(string) in only valid file mode");

    //TODO Display the string in the kernel terminal

    append_to_file(s.c_str(), s.size());
}

void logging::log(log_level /*level*/, const char* s){
    //First, print to the virtual debugger
    virtual_debug(s);

    if(is_early()){
        thor_assert(current_early < MAX_EARLY, "early log buffer is full");

        early_logs[current_early++] = s;
    }

    if(is_file()){
        append_to_file(s, std::str_len(s));
    }
}

void logging::log(log_level level, const std::string& s){
    thor_assert(!is_early(), "log(level,string) is not valid in early mode");

    log(level, s.c_str());
}

void logging::logf(log_level level, const char* s, ...){
    thor_assert(!is_early(), "logf(level,string,...) in not valid in early mode");

    va_list va;
    va_start(va, s);
    auto formatted = vsprintf(s, va);
    va_end(va);

    log(level, formatted.c_str());
}
