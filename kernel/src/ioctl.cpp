//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ioctl.hpp"
#include "errors.hpp"
#include "scheduler.hpp"
#include "logging.hpp"
#include "fs/devfs.hpp"

int64_t ioctl(size_t device_fd, ioctl_request request, void* data){
    if(!scheduler::has_handle(device_fd)){
        return -std::ERROR_INVALID_FILE_DESCRIPTOR;
    }

    auto& device = scheduler::get_handle(device_fd);

    if(request == ioctl_request::GET_BLK_SIZE){
        return devfs::get_device_size(device, *reinterpret_cast<size_t*>(data));
    }

    return std::ERROR_INVALID_REQUEST;
}
