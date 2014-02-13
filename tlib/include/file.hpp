//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_FILE_HPP
#define USER_FILE_HPP

#include <types.hpp>
#include <expected.hpp>
#include <string.hpp>
#include <stat_info.hpp>

std::expected<size_t> open(const char* file);
std::expected<size_t> read(size_t fd, char* buffer, size_t max);
void close(size_t fd);
std::expected<stat_info> stat(size_t fd);

#endif
