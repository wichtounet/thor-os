//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_IP_LAYER_H
#define NET_IP_LAYER_H

#include <types.hpp>

namespace network {

namespace ip {

struct address {
    uint32_t address = 0;

    uint8_t operator()(size_t index) const {
        return (address >> ((3 - index) * 8)) & 0xFF;
    }

    void set_sub(size_t index, uint8_t value){
        address |= uint32_t(value) << ((3 - index) * 8);
    }
};

} // end of ip namespace

} // end of network namespace

#endif
