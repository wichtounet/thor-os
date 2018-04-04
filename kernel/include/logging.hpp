//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <types.hpp>
#include <string.hpp>
#include <stdarg.h>

#ifndef LOGGING_HPP
#define LOGGING_HPP

namespace logging {

enum class log_level : char {
    TRACE,
    DEBUG,
    WARNING,
    ERROR,
    USER
};

bool is_early();
bool is_file();
void finalize();
void to_file();

void log(log_level level, const char* s);
void log(log_level level, const std::string& s);
void logf(log_level level, const char* s, va_list va);
void logf(log_level level, const char* s, ...);

void log(const char* s);
void log(const std::string& s);
void logf(const char* s, va_list va);
void logf(const char* s, ...);

} //end of namespace logging

#endif
