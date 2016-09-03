//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <types.hpp>

#include "tlib/net_constants.hpp"

#include "assert.hpp"

#include "net/ethernet_packet.hpp"

namespace network {

struct socket {
    size_t id;
    socket_domain domain;
    socket_type type;
    socket_protocol protocol;
    size_t next_fd;

    std::vector<network::ethernet::packet> packets;

    //socket(){}
    //socket(size_t id, socket_domain domain, socket_type type, socket_protocol protocol)
            //: id(id), domain(domain), type(type), protocol(protocol) {}

    void invalidate(){
        id = 0xFFFFFFFF;
    }

    bool is_valid() const {
        return id != 0xFFFFFFFF;
    }

    size_t register_packet(network::ethernet::packet packet){
        auto fd = next_fd++;

        packet.fd = fd;

        packets.push_back(packet);

        return fd;
    }

    bool has_packet(size_t packet_fd){
        for(auto& packet : packets){
            if(packet.fd == packet_fd){
                return true;
            }
        }

        return false;
    }

    network::ethernet::packet& get_packet(size_t fd){
        for(auto& packet : packets){
            if(packet.fd == fd){
                return packet;
            }
        }

        thor_unreachable("Should not happen");
    }

    void erase_packet(size_t fd){
        for(size_t i = 0; i < packets.size(); ++i){
            if(packets[i].fd == fd){
                packets.erase(i);
                return;
            }
        }

        thor_unreachable("Should not happen");
    }
};

} // end of network namespace

#endif
