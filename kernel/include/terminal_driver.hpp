//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TERMINAL_DRIVER_H
#define TERMINAL_DRIVER_H

#include <types.hpp>

#include "fs/devfs.hpp"

namespace stdio {

struct terminal_driver final : devfs::char_driver {
    size_t read(void* data, char* buffer, size_t count, size_t& read) override;
    size_t read(void* data, char* buffer, size_t count, size_t& read, size_t ms) override;
    size_t write(void* data, const char* buffer, size_t count, size_t& written) override;
};

} //end of namespace stdio

#endif
