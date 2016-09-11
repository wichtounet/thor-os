//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
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
#include <statfs_info.hpp>

std::expected<size_t> open(const char* file, size_t flags = 0);
int64_t mkdir(const char* file);
int64_t rm(const char* file);
std::expected<size_t> read(size_t fd, char* buffer, size_t max, size_t offset = 0);
std::expected<size_t> write(size_t fd, const char* buffer, size_t max, size_t offset = 0);
std::expected<size_t> truncate(size_t fd, size_t size);
std::expected<size_t> entries(size_t fd, char* buffer, size_t max);
void close(size_t fd);
std::expected<stat_info> stat(size_t fd);
std::expected<statfs_info> statfs(const char* file);
std::expected<size_t> mounts(char* buffer, size_t max);

std::string current_working_directory();
void set_current_working_directory(const std::string& directory);

#endif
