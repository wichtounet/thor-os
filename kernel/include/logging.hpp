//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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

} //end of namespace logging

#endif
