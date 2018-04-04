//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_CONNECTION_HANDLER_H
#define NET_CONNECTION_HANDLER_H

#include <list.hpp>

#include "conc/rw_lock.hpp"

namespace network {

/*!
 * \brief A thread-safe collection of network connection (UDP/TCP)
 */
template <typename C>
struct connection_handler {
    using connection_type = C; ///< The type of connnection

    /*!
     * \brief Get the first connection matching the packet ports
     */
    connection_type* get_connection_for_packet(size_t source_port, size_t target_port) {
        auto lock = connections_lock.reader_lock();
        std::lock_guard<reader_rw_lock> l(lock);

        for (auto& connection : connections) {
            if(connection.server){
                if (connection.server_port == target_port) {
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

    /*!
     * \brief Execute a functor for each connection matcing the packet ports
     */
    template<typename Functor>
    void for_each_connection_for_packet(size_t source_port, size_t target_port, Functor fun){
        auto lock = connections_lock.reader_lock();
        std::lock_guard<reader_rw_lock> l(lock);

        for (auto& connection : connections) {
            if(connection.server){
                if (connection.server_port == target_port) {
                    fun(connection);
                }
            } else {
                if (connection.server_port == source_port && connection.local_port == target_port) {
                    fun(connection);
                }
            }
        }
    }

    /*!
     * \brief Create a new connection
     */
    connection_type& create_connection() {
        auto lock = connections_lock.writer_lock();
        std::lock_guard<writer_rw_lock> l(lock);

        return connections.emplace_back();
    }

    /*!
     * \brief Remove the connection from the collection
     */
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
    rw_lock connections_lock; ///< The Readers/Writer lock

    std::list<connection_type> connections; ///< The list of connections
};

} // end of network namespace

#endif
