//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP

#include <types.hpp>
#include <expected.hpp>
#include <string.hpp>

std::expected<size_t> open(const char* file);

#endif
