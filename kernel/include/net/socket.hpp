//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <types.hpp>
#include <vector.hpp>
#include <algorithms.hpp>
#include <circular_buffer.hpp>

#include "tlib/net_constants.hpp"

#include "conc/sleep_queue.hpp"

#include "net/ethernet_packet.hpp"

#include "assert.hpp"

namespace network {

struct socket {
    size_t id;
    socket_domain domain;
    socket_type type;
    socket_protocol protocol;
    size_t next_fd;
    bool listen;

    bool connected;
    uint32_t local_port;
    uint32_t server_port;
    ip::address server_address;
    uint32_t ack_number;
    uint32_t seq_number;

    std::vector<network::ethernet::packet> packets;

    circular_buffer<network::ethernet::packet, 32> listen_packets;
    sleep_queue listen_queue;

    socket(){}
    socket(size_t id, socket_domain domain, socket_type type, socket_protocol protocol, size_t next_fd, bool listen)
            : id(id), domain(domain), type(type), protocol(protocol), next_fd(next_fd), listen(listen) {}

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
        packets.erase(std::remove_if(packets.begin(), packets.end(), [fd](network::ethernet::packet& packet){
            return packet.fd == fd;
        }), packets.end());
    }
};

} // end of network namespace

#endif
