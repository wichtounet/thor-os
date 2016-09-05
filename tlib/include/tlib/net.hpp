//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef TLIB_NET_H
#define TLIB_NET_H

#include <expected.hpp>

#include "tlib/net_constants.hpp"
#include "tlib/config.hpp"

ASSERT_ONLY_THOR_PROGRAM

namespace tlib {

struct packet {
    size_t fd;
    char* payload;
    size_t index;
};

std::expected<size_t> socket_open(socket_domain domain, socket_type type, socket_protocol protocol);
void socket_close(size_t socket_fd);

std::expected<packet> prepare_packet(size_t socket_fd, void* desc);
std::expected<void> finalize_packet(size_t socket_fd, packet p);
std::expected<void> listen(size_t socket_fd, bool l);
std::expected<packet> wait_for_packet(size_t socket_fd);
void release_packet(packet& packet);

} // end of namespace tlib

#endif
