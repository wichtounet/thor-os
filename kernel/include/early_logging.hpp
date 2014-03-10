//=======================================================================
// Copyright Baptiste Wicht 2013-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <types.hpp>

#ifndef EARLY_LOGGING_HPP
#define EARLY_LOGGING_HPP

namespace early {

constexpr const uint32_t MAX_EARLY_LOGGING = 128;

extern uint32_t early_logs_count;
extern uint32_t early_logs[MAX_EARLY_LOGGING];

} //end of namespace early

#endif