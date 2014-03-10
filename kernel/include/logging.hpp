//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <types.hpp>
#include <string.hpp>

#ifndef LOGGING_HPP
#define LOGGING_HPP

namespace logging {

bool is_early();
bool is_file();
void finalize();
void to_file();

void log(const char* s);
void log(const std::string& s);

} //end of namespace logging

#endif