//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>
#include <atomic.hpp>
#include <list.hpp>

#include "conc/condition_variable.hpp"
#include "conc/rw_lock.hpp"

#include "net/tcp_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/checksum.hpp"

#include "tlib/errors.hpp"

#include "kernel_utils.hpp"
#include "circular_buffer.hpp"
#include "timer.hpp"

namespace {

static constexpr size_t timeout_ms = 1000;
static constexpr size_t max_tries  = 5;

using flag_data_offset = std::bit_field<uint16_t, uint8_t, 12, 4>;
using flag_reserved    = std::bit_field<uint16_t, uint8_t, 9, 3>;
using flag_ns          = std::bit_field<uint16_t, uint8_t, 8, 1>;
using flag_cwr         = std::bit_field<uint16_t, uint8_t, 7, 1>;
using flag_ece         = std::bit_field<uint16_t, uint8_t, 6, 1>;
using flag_urg         = std::bit_field<uint16_t, uint8_t, 5, 1>;
using flag_ack         = std::bit_field<uint16_t, uint8_t, 4, 1>;
using flag_psh         = std::bit_field<uint16_t, uint8_t, 3, 1>;
using flag_rst         = std::bit_field<uint16_t, uint8_t, 2, 1>;
using flag_syn         = std::bit_field<uint16_t, uint8_t, 1, 1>;
using flag_fin         = std::bit_field<uint16_t, uint8_t, 0, 1>;

struct tcp_connection {
    size_t local_port;                   ///< The local source port
    size_t server_port;                  ///< The server port
    network::ip::address server_address; ///< The server address

    std::atomic<bool> listening;                           ///< Indicates if a kernel thread is listening on this connection
    condition_variable queue;                              ///< The listening queue
    circular_buffer<network::ethernet::packet, 8> packets; ///< The packets for the listening queue

    bool connected = false;

    uint32_t ack_number = 0; ///< The next ack number
    uint32_t seq_number = 0; ///< The next sequence number

    network::socket* socket = nullptr;
};

// The lock used to protect the list of connections
rw_lock connections_lock;

// Note: We need a list to not invalidate the values during insertions
std::list<tcp_connection> connections;

tcp_connection* get_connection_for_packet(size_t source_port, size_t target_port) {
    auto lock = connections_lock.reader_lock();
    std::lock_guard<reader_rw_lock> l(lock);

    for(auto& connection : connections){
        if (connection.server_port == source_port && connection.local_port == target_port) {
            return &connection;
        }
    }

    return nullptr;
}

tcp_connection& create_connection(){
    auto lock = connections_lock.writer_lock();
    std::lock_guard<writer_rw_lock> l(lock);

    auto& connection = connections.emplace_back();

    connection.listening = false;

    return connection;
}

void remove_connection(tcp_connection& connection){
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

void compute_checksum(network::ethernet::packet& packet) {
    auto* ip_header  = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    auto tcp_len = switch_endian_16(ip_header->total_len) - sizeof(network::ip::header);

    tcp_header->checksum = 0;

    // Accumulate the Payload
    auto sum = network::checksum_add_bytes(packet.payload + packet.index, tcp_len);

    // Accumulate the IP addresses
    sum += network::checksum_add_bytes(&ip_header->source_ip, 8);

    // Accumulate the IP Protocol
    sum += ip_header->protocol;

    // Accumulate the TCP length
    sum += tcp_len;

    // Complete the 1-complement sum
    tcp_header->checksum = switch_endian_16(network::checksum_finalize_nz(sum));
}

uint16_t get_default_flags() {
    uint16_t flags = 0; // By default

    (flag_data_offset(&flags)) = 5; // No options

    return flags;
}

void prepare_packet(network::ethernet::packet& packet, size_t source, size_t target) {
    packet.tag(2, packet.index);

    // Set the TCP header

    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    tcp_header->source_port = switch_endian_16(source);
    tcp_header->target_port = switch_endian_16(target);
    tcp_header->window_size = 1024;

    packet.index += sizeof(network::tcp::header);
}

size_t tcp_payload_len(const network::ethernet::packet& packet){
    auto* ip_header  = reinterpret_cast<const network::ip::header*>(packet.payload + packet.tag(1));
    auto* tcp_header = reinterpret_cast<const network::tcp::header*>(packet.payload + packet.tag(2));

    auto tcp_flags = switch_endian_16(tcp_header->flags);

    auto ip_header_len   = (ip_header->version_ihl & 0xF) * 4;
    auto tcp_len         = switch_endian_16(ip_header->total_len) - ip_header_len;
    auto tcp_data_offset = *flag_data_offset(&tcp_flags) * 4;
    auto payload_len     = tcp_len - tcp_data_offset;

    return payload_len;
}

} //end of anonymous namespace

void network::tcp::decode(network::interface_descriptor& interface, network::ethernet::packet& packet) {
    packet.tag(2, packet.index);

    auto* ip_header = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "tcp: Start TCP packet handling\n");

    auto source_port = switch_endian_16(tcp_header->source_port);
    auto target_port = switch_endian_16(tcp_header->target_port);
    auto seq         = switch_endian_32(tcp_header->sequence_number);
    auto ack         = switch_endian_32(tcp_header->ack_number);

    logging::logf(logging::log_level::TRACE, "tcp: Source Port %u \n", size_t(source_port));
    logging::logf(logging::log_level::TRACE, "tcp: Target Port %u \n", size_t(target_port));
    logging::logf(logging::log_level::TRACE, "tcp: Seq Number %u \n", size_t(seq));
    logging::logf(logging::log_level::TRACE, "tcp: Ack Number %u \n", size_t(ack));

    auto flags = switch_endian_16(tcp_header->flags);

    auto next_seq = ack;
    auto next_ack = seq + tcp_payload_len(packet);;

    auto connection_ptr = get_connection_for_packet(source_port, target_port);

    if(connection_ptr){
        auto& connection = *connection_ptr;

        // Update the connection status

        connection.seq_number = next_seq;
        connection.ack_number = next_ack;

        // Propagate to kernel connections

        if (connection.listening.load()) {
            auto copy    = packet;
            copy.payload = new char[copy.payload_size];
            std::copy_n(packet.payload, packet.payload_size, copy.payload);

            connection.packets.push(copy);
            connection.queue.notify_one();
        }

        // Propagate to the kernel socket

        if (*flag_psh(&flags) && connection.socket) {
            auto& socket = *connection.socket;

            packet.index += sizeof(header);

            if (socket.listen) {
                auto copy    = packet;
                copy.payload = new char[copy.payload_size];
                std::copy_n(packet.payload, packet.payload_size, copy.payload);

                socket.listen_packets.push(copy);
                socket.listen_queue.notify_one();
            }
        }
    } else {
        logging::logf(logging::log_level::DEBUG, "tcp: Received packet for which there are no connection\n");
    }

    // Acknowledge if necessary

    if (*flag_psh(&flags)) {
        auto p = tcp::prepare_packet(interface, switch_endian_32(ip_header->source_ip), target_port, source_port, 0);

        if (!p) {
            logging::logf(logging::log_level::ERROR, "tcp: Impossible to prepare TCP packet for ACK\n");
            return;
        }

        auto* ack_tcp_header = reinterpret_cast<header*>(p->payload + p->tag(2));

        ack_tcp_header->sequence_number = switch_endian_32(next_seq);
        ack_tcp_header->ack_number      = switch_endian_32(next_ack);

        auto ack_flags = get_default_flags();
        (flag_ack(&ack_flags)) = 1;
        ack_tcp_header->flags = switch_endian_16(ack_flags);

        tcp::finalize_packet(interface, *p);
    }
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size) {
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(interface, sizeof(header) + payload_size, target_ip, 0x06);

    if (packet) {
        ::prepare_packet(*packet, source, target);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(char* buffer, network::socket& socket, size_t payload_size) {
    auto& connection = socket.get_data<tcp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<network::ethernet::packet>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    auto target_ip  = connection.server_address;
    auto& interface = network::select_interface(target_ip);

    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(buffer, interface, sizeof(header) + payload_size, target_ip, 0x06);

    if (packet) {
        auto source = connection.local_port;
        auto target = connection.server_port;

        ::prepare_packet(*packet, source, target);

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        auto flags = get_default_flags();
        (flag_psh(&flags)) = 1;
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        tcp_header->sequence_number = switch_endian_32(connection.seq_number);
        tcp_header->ack_number      = switch_endian_32(connection.ack_number);
    }

    return packet;
}

void network::tcp::finalize_packet(network::interface_descriptor& interface, network::ethernet::packet& p) {
    p.index -= sizeof(header);

    // Compute the checksum
    compute_checksum(p);

    // Give the packet to the IP layer for finalization
    network::ip::finalize_packet(interface, p);
}

void network::tcp::finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::ethernet::packet& p) {
    p.index -= sizeof(header);

    // Compute the checksum
    compute_checksum(p);

    if (!p.user) {
        logging::logf(logging::log_level::ERROR, "tcp: Function uniquely implemented for user packets!\n");
        return; //TODO Fail
    }

    auto& connection = socket.get_data<tcp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        //TODO return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
        return;
    }

    connection.listening = true;

    uint32_t seq = 0;
    uint32_t ack = 0;

    bool received = false;

    for(size_t t = 0; t < max_tries; ++t){
        // Give the packet to the IP layer for finalization
        network::ip::finalize_packet(interface, p);

        auto before = timer::milliseconds();
        auto after  = before;

        while(true){
            // Make sure we don't wait for more than the timeout
            if (after > before + timeout_ms) {
                break;
            }

            auto remaining = timeout_ms - (after - before);

            if(connection.queue.wait_for(remaining)){
                auto received_packet = connection.packets.pop();

                auto* tcp_header = reinterpret_cast<header*>(received_packet.payload + received_packet.index);
                auto flags       = switch_endian_16(tcp_header->flags);

                if (*flag_ack(&flags)) {
                    logging::logf(logging::log_level::TRACE, "tcp: Received ACK\n");

                    seq = switch_endian_32(tcp_header->sequence_number);
                    ack = switch_endian_32(tcp_header->ack_number);

                    delete[] received_packet.payload;

                    received = true;

                    break;
                } else {
                    logging::logf(logging::log_level::TRACE, "tcp: Received unrelated answer\n");
                }

                delete[] received_packet.payload;
            }
        }

        if(received){
            break;
        }

        after = timer::milliseconds();
    }

    // Stop listening
    connection.listening = false;

    if(received){
        // Set the future sequence and acknowledgement numbers
        connection.seq_number = ack;
        connection.ack_number = seq;
    } else {
        //TODO We need to be able to make finalize fail!
    }
}

std::expected<void> network::tcp::connect(network::socket& sock, network::interface_descriptor& interface, size_t local_port, size_t server_port, network::ip::address server) {
    // Create the connection

    auto& connection = create_connection();

    connection.local_port     = local_port;
    connection.server_port    = server_port;
    connection.server_address = server;

    // Link the socket and connection
    sock.data = &connection;
    connection.socket = &sock;

    // Prepare the SYN packet

    auto target_ip = connection.server_address;

    auto packet = tcp::prepare_packet(interface, target_ip, local_port, server_port, 0);

    if (!packet) {
        return std::make_unexpected<void>(packet.error());
    }

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    auto flags = get_default_flags();
    (flag_syn(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    tcp_header->sequence_number = connection.seq_number;
    tcp_header->ack_number      = connection.ack_number;

    connection.listening = true;

    logging::logf(logging::log_level::TRACE, "tcp: Send SYN\n");
    tcp::finalize_packet(interface, *packet);

    uint32_t seq;
    uint32_t ack;

    while (true) {
        // TODO Need a timeout
        connection.queue.wait();
        auto received_packet = connection.packets.pop();

        auto* tcp_header = reinterpret_cast<header*>(received_packet.payload + received_packet.index);
        auto flags       = switch_endian_16(tcp_header->flags);

        logging::logf(logging::log_level::TRACE, "tcp: Received answer\n");

        if (*flag_syn(&flags) && *flag_ack(&flags)) {
            logging::logf(logging::log_level::TRACE, "tcp: Received SYN/ACK\n");

            seq = switch_endian_32(tcp_header->sequence_number);
            ack = switch_endian_32(tcp_header->ack_number);

            delete[] received_packet.payload;

            break;
        }

        delete[] received_packet.payload;
    }

    connection.listening = false;

    connection.seq_number = ack;
    connection.ack_number = seq + 1;

    // At this point we have received the SYN/ACK, only remains to ACK

    {
        auto packet = tcp::prepare_packet(interface, target_ip, connection.local_port, connection.server_port, 0);

        if (!packet) {
            return std::make_unexpected<void>(packet.error());
        }

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        tcp_header->sequence_number = switch_endian_32(connection.seq_number);
        tcp_header->ack_number      = switch_endian_32(connection.ack_number);

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp: Send ACK\n");
        tcp::finalize_packet(interface, *packet);
    }

    // Mark the connection as connected

    connection.connected = true;

    return {};
}

std::expected<void> network::tcp::disconnect(network::socket& sock) {
    auto& connection = sock.get_data<tcp_connection>();

    if(!connection.connected){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    auto target_ip = connection.server_address;
    auto source    = connection.local_port;
    auto target    = connection.server_port;

    auto& interface = network::select_interface(target_ip);

    auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

    if (!packet) {
        return std::make_unexpected<void>(packet.error());
    }

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    tcp_header->sequence_number = switch_endian_32(connection.seq_number);
    tcp_header->ack_number      = switch_endian_32(connection.ack_number);

    auto flags = get_default_flags();
    (flag_fin(&flags)) = 1;
    (flag_ack(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    connection.listening = true;

    logging::logf(logging::log_level::TRACE, "tcp: Send FIN/ACK\n");
    tcp::finalize_packet(interface, *packet);

    uint32_t seq;
    uint32_t ack;

    auto fin_rec     = false;
    auto fin_ack_rec = false;

    while (true) {
        // TODO Need a timeout
        connection.queue.wait();
        auto received_packet = connection.packets.pop();

        auto* tcp_header = reinterpret_cast<header*>(received_packet.payload + received_packet.index);
        auto flags       = switch_endian_16(tcp_header->flags);

        logging::logf(logging::log_level::TRACE, "tcp: Received answer\n");

        if (*flag_fin(&flags) && *flag_ack(&flags)) {
            logging::logf(logging::log_level::TRACE, "tcp: Received FIN/ACK\n");

            seq = switch_endian_32(tcp_header->sequence_number);
            ack = switch_endian_32(tcp_header->ack_number);

            fin_ack_rec = true;
        } else if (*flag_ack(&flags)) {
            logging::logf(logging::log_level::TRACE, "tcp: Received ACK\n");

            seq = switch_endian_32(tcp_header->sequence_number);
            ack = switch_endian_32(tcp_header->ack_number);

            fin_rec = true;
        }

        delete[] received_packet.payload;

        if (fin_rec && fin_ack_rec) {
            break;
        }
    }

    connection.listening = false;

    connection.seq_number = ack;
    connection.ack_number = seq + 1;

    // At this point we have received the FIN/ACK, only remains to ACK

    {
        auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

        if (!packet) {
            return std::make_unexpected<void>(packet.error());
        }

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        tcp_header->sequence_number = switch_endian_32(connection.seq_number);
        tcp_header->ack_number      = switch_endian_32(connection.ack_number);

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp: Send ACK\n");
        tcp::finalize_packet(interface, *packet);
    }

    // Mark the connection as connected

    connection.connected = false;

    remove_connection(connection);

    return {};
}
