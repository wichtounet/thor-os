//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_ETHERNET_PACKET_H
#define NET_ETHERNET_PACKET_H

#include <types.hpp>

#include "assert.hpp"

namespace network {

namespace ethernet {

struct packet {
    // Set from the beginning
    char* payload;
    size_t payload_size;

    // Set by ethernet
    ether_type type;
    size_t index;

    // Set for user mode
    size_t fd;
    bool user;

    uint64_t tags; // This allows for 4 tags (4 layer)
    uint64_t interface; ///< Id of the interface

    packet() : fd(0), user(false), tags(0) {}
    packet(char* payload, size_t payload_size) : payload(payload), payload_size(payload_size), index(0), fd(0), user(false), tags(0) {}

    void tag(size_t layer, size_t index){
        tags |= (index << (layer * 16));
    }

    size_t tag(size_t layer) const {
        thor_assert(layer < 4, "Invalid tag access");
        return (tags >> (layer * 16)) & 0xFFFF;
    }
};

} // end of ethernet namespace

} // end of network namespace

#endif
