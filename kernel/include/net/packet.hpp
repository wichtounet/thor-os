//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_PACKET_H
#define NET_PACKET_H

#include <types.hpp>
#include <shared_ptr.hpp>

#include "assert.hpp"

namespace network {

/*!
 * \brief A network packet.
 */
struct packet {
    // Set from the beginning
    char* payload;          ///< Pointer to the packet payload
    size_t payload_size;    ///< The size, in bytes, of the payload
    size_t index;           ///< The current index inside the payload

    // Set for user mode
    size_t fd; ///< The file descriptor (in user mode)
    bool user; ///< Flag indicating if the packet is a user packet or kernel packet

    uint64_t tags;          ///< Tags of the layer indices
    uint64_t interface;     ///< Id of the interface

    packet() : fd(0), user(false), tags(0) {}
    packet(char* payload, size_t payload_size) : payload(payload), payload_size(payload_size), index(0), fd(0), user(false), tags(0) {}

    packet(const packet& rhs) = delete;
    packet& operator=(const packet& rhs) = delete;

    packet(const packet&& rhs) = delete;
    packet& operator=(const packet&& rhs) = delete;

    ~packet(){
        if(!user && payload){
            delete[] payload;
        }
    }

    void tag(size_t layer, size_t index){
        tags |= (index << (layer * 16));
    }

    size_t tag(size_t layer) const {
        thor_assert(layer < 4, "Invalid tag access");
        return (tags >> (layer * 16)) & 0xFFFF;
    }
};

using packet_p = std::shared_ptr<packet>;

} // end of network namespace

#endif
