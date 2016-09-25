//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_CONNECTION_HANDLER_H
#define NET_CONNECTION_HANDLER_H

#include <list.hpp>

#include "conc/rw_lock.hpp"

namespace network {

template <typename C>
struct connection_handler {
    using connection_type = C;

    connection_type* get_connection_for_packet(size_t source_port, size_t target_port) {
        auto lock = connections_lock.reader_lock();
        std::lock_guard<reader_rw_lock> l(lock);

        for (auto& connection : connections) {
            if(connection.server){
                if (connection.server_port == target_port && connection.local_port == source_port) {
                    return &connection;
                }
            } else {
                if (connection.server_port == source_port && connection.local_port == target_port) {
                    return &connection;
                }
            }
        }

        return nullptr;
    }

    connection_type& create_connection() {
        auto lock = connections_lock.writer_lock();
        std::lock_guard<writer_rw_lock> l(lock);

        return connections.emplace_back();
    }

    void remove_connection(connection_type& connection) {
        auto lock = connections_lock.writer_lock();
        std::lock_guard<writer_rw_lock> l(lock);

        auto end = connections.end();
        auto it  = connections.begin();

        while (it != end) {
            if (&(*it) == &connection) {
                connections.erase(it);
                return;
            }

            ++it;
        }
    }

private:
    rw_lock connections_lock;

    std::list<connection_type> connections;
};

} // end of network namespace

#endif
