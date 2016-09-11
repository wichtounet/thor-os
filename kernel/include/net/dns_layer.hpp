//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_DNS_LAYER_H
#define NET_DNS_LAYER_H

#include <types.hpp>

#include "net/ethernet_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/network.hpp"

namespace network {

namespace dns {

struct flags_t {
    uint8_t qr : 1;
    uint8_t opcode : 4;
    uint8_t aa : 1;
    uint8_t tc : 1;
    uint8_t rd : 1;
    uint8_t ra : 1;
    uint8_t zeroes : 3;
    uint8_t rcode : 4;
} __attribute__((packed));

static_assert(sizeof(flags_t) == 2, "DNS flags must be 16 bits");

struct header {
    uint16_t identification;
    flags_t flags;
    uint16_t questions;
    uint16_t answers;
    uint16_t authority_rrs;
    uint16_t additional_rrs;
} __attribute__((packed));

void decode(network::interface_descriptor& interface, network::ethernet::packet& packet);

} // end of dns namespace

} // end of network namespace

#endif
