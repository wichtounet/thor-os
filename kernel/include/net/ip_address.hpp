//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_IP_ADDRESS_H
#define NET_IP_ADDRESS_H

#include <types.hpp>

namespace network {

namespace ip {

//TODO Maybe packed 4x8 is better

struct address {
    uint32_t raw_address = 0;

    address(){}
    address(uint32_t raw) : raw_address(raw) {}

    uint8_t operator()(size_t index) const {
        return (raw_address >> ((3 - index) * 8)) & 0xFF;
    }

    void set_sub(size_t index, uint8_t value){
        raw_address |= uint32_t(value) << ((3 - index) * 8);
    }

    bool operator==(const address& rhs) const {
        return this->raw_address == rhs.raw_address;
    }
};

inline address make_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
    address addr;
    addr.set_sub(0, a);
    addr.set_sub(1, b);
    addr.set_sub(2, c);
    addr.set_sub(3, d);
    return addr;
}

} // end of ip namespace

} // end of network namespace

#endif
