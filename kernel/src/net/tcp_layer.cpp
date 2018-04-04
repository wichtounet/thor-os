//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>

#include "tlib/errors.hpp"

#include "net/tcp_layer.hpp"
#include "net/ip_layer.hpp"
#include "net/checksum.hpp"
#include "net/network.hpp"

#include "kernel_utils.hpp"
#include "timer.hpp"

namespace {

static constexpr size_t timeout_ms = 1000;
static constexpr size_t max_tries  = 5;
constexpr size_t default_tcp_header_length = 20;

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

void compute_checksum(network::packet& packet) {
    auto* ip_header  = reinterpret_cast<network::ip::header*>(packet.payload + packet.tag(1));
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.tag(2));

    auto tcp_len = switch_endian_16(ip_header->total_len) - (ip_header->version_ihl & 0xF) * 4;

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

    (flag_data_offset(&flags)) = default_tcp_header_length / 4; // No options

    return flags;
}

void prepare_packet(network::packet& packet, size_t source, size_t target) {
    packet.tag(2, packet.index);

    // Set the TCP header

    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet.payload + packet.index);

    tcp_header->source_port    = switch_endian_16(source);
    tcp_header->target_port    = switch_endian_16(target);
    tcp_header->window_size    = switch_endian_16(1024);
    tcp_header->urgent_pointer = 0;

    packet.index += default_tcp_header_length;
}

size_t tcp_data_offset(const network::packet_p& packet){
    auto* tcp_header = reinterpret_cast<const network::tcp::header*>(packet->payload + packet->tag(2));
    auto tcp_flags = switch_endian_16(tcp_header->flags);
    return *flag_data_offset(&tcp_flags) * 4;
}

size_t tcp_payload_len(const network::packet_p& packet){
    auto* ip_header  = reinterpret_cast<const network::ip::header*>(packet->payload + packet->tag(1));
    auto* tcp_header = reinterpret_cast<const network::tcp::header*>(packet->payload + packet->tag(2));

    auto tcp_flags = switch_endian_16(tcp_header->flags);

    auto ip_header_len   = (ip_header->version_ihl & 0xF) * 4;
    auto tcp_len         = switch_endian_16(ip_header->total_len) - ip_header_len;
    auto tcp_data_offset = *flag_data_offset(&tcp_flags) * 4;
    auto payload_len     = tcp_len - tcp_data_offset;

    return payload_len;
}

} //end of anonymous namespace

network::tcp::layer::layer(network::ip::layer* parent) : parent(parent) {
    parent->register_tcp_layer(this);

    // The first port will be 1024
    local_port = 1023;
}

void network::tcp::layer::decode(network::interface_descriptor& interface, network::packet_p& packet) {
    packet->tag(2, packet->index);

    auto* ip_header = reinterpret_cast<network::ip::header*>(packet->payload + packet->tag(1));
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet->payload + packet->index);

    logging::logf(logging::log_level::TRACE, "tcp:decode: Start TCP packet handling (%p)\n", packet.get());

    auto source_port = switch_endian_16(tcp_header->source_port);
    auto target_port = switch_endian_16(tcp_header->target_port);
    auto seq         = switch_endian_32(tcp_header->sequence_number);
    auto ack         = switch_endian_32(tcp_header->ack_number);
    auto len         = tcp_payload_len(packet);

    logging::logf(logging::log_level::TRACE, "tcp:decode: Source Port %u \n", size_t(source_port));
    logging::logf(logging::log_level::TRACE, "tcp:decode: Target Port %u \n", size_t(target_port));
    logging::logf(logging::log_level::TRACE, "tcp:decode: Seq Number %u \n", size_t(seq));
    logging::logf(logging::log_level::TRACE, "tcp:decode: Ack Number %u \n", size_t(ack));
    logging::logf(logging::log_level::TRACE, "tcp:decode: Length %u \n", size_t(len));

    auto flags = switch_endian_16(tcp_header->flags);

    const bool is_syn = *flag_syn(&flags);
    const bool is_ack = *flag_ack(&flags);
    const bool is_fin = *flag_fin(&flags);
    const bool is_psh = *flag_psh(&flags);

    logging::logf(logging::log_level::TRACE, "tcp:decode: SYN:%b ACK:%b FIN:%b PSH:%b \n", is_syn, is_ack, is_fin, is_psh);

    auto next_seq = ack;
    auto next_ack = seq + len;

    logging::logf(logging::log_level::TRACE, "tcp:decode: Next Seq Number %u \n", size_t(next_seq));
    logging::logf(logging::log_level::TRACE, "tcp:decode: Next Ack Number %u \n", size_t(next_ack));

    connections.for_each_connection_for_packet(source_port, target_port, [&](tcp_connection& connection) {
        if(connection.socket){
            logging::logf(logging::log_level::TRACE, "tcp:decode: Found connection with socket\n");
        } else {
            logging::logf(logging::log_level::TRACE, "tcp:decode: Found connection without socket\n");
        }

        logging::logf(logging::log_level::TRACE, "tcp:decode: connection (server:%b,connected:%b,child:%b)\n", connection.server, connection.connected, connection.child);
        logging::logf(logging::log_level::TRACE, "            src:%u dest:%u server:%h\n", connection.local_port, connection.server_port, connection.server_address.raw_address);

        // Update the connection status

        connection.seq_number = next_seq;
        connection.ack_number = next_ack;

        // Propagate to kernel connections

        if (connection.listening.load()) {
            logging::logf(logging::log_level::TRACE, "tcp:decode: Propagated to connection\n");

            connection.packets.push(packet);
            connection.queue.notify_one();
        }

        if(connection.child && is_fin){
            logging::logf(logging::log_level::TRACE, "tcp:decode: End connection (received FIN/ACK)\n");

            logging::logf(logging::log_level::TRACE, "tcp:decode: Send FIN/ACK\n");

            auto p = kernel_prepare_packet(interface, switch_endian_32(ip_header->source_ip), target_port, source_port, 0);

            if (!p) {
                logging::logf(logging::log_level::ERROR, "tcp:decode: Impossible to prepare TCP packet for FIN/ACK\n");
            } else {
                auto& packet = *p;

                auto* ack_tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

                ack_tcp_header->sequence_number = switch_endian_32(connection.seq_number);
                ack_tcp_header->ack_number      = switch_endian_32(connection.ack_number);

                auto ack_flags = get_default_flags();
                (flag_fin(&ack_flags)) = 1;
                (flag_ack(&ack_flags)) = 1;
                ack_tcp_header->flags = switch_endian_16(ack_flags);

                finalize_packet_direct(interface, packet);
            }

            //Wake up the thread (if any) waiting on this connection

            connection.connected = false;
            connection.queue.notify_all();

            if(connection.socket){
                connection.socket->listen_queue.notify_all();
            }
        }

        // Propagate to the kernel socket

        if (is_psh && connection.socket) {
            auto& socket = *connection.socket;

            packet->index += *flag_data_offset(&flags) * 4;

            if (socket.listen) {
                logging::logf(logging::log_level::TRACE, "tcp:decode: Propagated to socket\n");

                socket.listen_packets.push(packet);
                socket.listen_queue.notify_one();
            }
        }
    });

    // Acknowledge if necessary

    if (is_psh) {
        logging::logf(logging::log_level::TRACE, "tcp:decode: Acknowledge directly\n");

        auto p = kernel_prepare_packet(interface, switch_endian_32(ip_header->source_ip), target_port, source_port, 0);

        if (!p) {
            logging::logf(logging::log_level::ERROR, "tcp:decode: Impossible to prepare TCP packet for ACK\n");
            return;
        }

        auto& packet = *p;

        auto* ack_tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        ack_tcp_header->sequence_number = switch_endian_32(next_seq);
        ack_tcp_header->ack_number      = switch_endian_32(next_ack);

        auto ack_flags = get_default_flags();
        (flag_ack(&ack_flags)) = 1;
        ack_tcp_header->flags = switch_endian_16(ack_flags);

        finalize_packet_direct(interface, packet);
    }

    logging::logf(logging::log_level::TRACE, "tcp:decode: Done\n");
}

std::expected<void> network::tcp::layer::send(char* target_buffer, network::socket& socket, const char* buffer, size_t n){
    auto& connection = socket.get_connection_data<tcp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<void>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    logging::logf(logging::log_level::TRACE, "tcp:send: Send %s(%u)\n", buffer, n);

    network::tcp::packet_descriptor desc{n};
    auto p = user_prepare_packet(target_buffer, socket, &desc);

    if (p) {
        auto& packet = *p;

        for(size_t i = 0; i < n; ++i){
            packet->payload[packet->index + i] = buffer[i];
        }

        auto target_ip  = connection.server_address;
        auto& interface = network::select_interface(target_ip);
        return finalize_packet(interface, socket, packet);
    }

    return std::make_unexpected<void>(p.error());
}

std::expected<size_t> network::tcp::layer::receive(char* buffer, network::socket& socket, size_t n){
    auto& connection = socket.get_connection_data<tcp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    logging::logf(logging::log_level::TRACE, "tcp:receive: Wait for packet\n");

    if(socket.listen_packets.empty()){
        socket.listen_queue.wait();

        if(!connection.connected){
            logging::logf(logging::log_level::TRACE, "tcp:receive: Disconnected while waiting\n");
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
        }
    }

    auto packet = socket.listen_packets.top();
    socket.listen_packets.pop();

    logging::logf(logging::log_level::TRACE, "tcp:receive: Received packet\n");

    auto payload_len = tcp_payload_len(packet);

    if(payload_len > n){
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    auto offset = packet->tag(2) + tcp_data_offset(packet);

    std::copy_n(packet->payload + offset, payload_len, buffer);

    return payload_len;
}

std::expected<size_t> network::tcp::layer::receive(char* buffer, network::socket& socket, size_t n, size_t ms){
    auto& connection = socket.get_connection_data<tcp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    logging::logf(logging::log_level::TRACE, "tcp:receive: Wait for packet (timeout)\n");

    if(socket.listen_packets.empty()){
        if(!ms){
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }

        if(!socket.listen_queue.wait_for(ms)){
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }

        if(!connection.connected){
            logging::logf(logging::log_level::TRACE, "tcp:receive: Disconnected while waiting\n");
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
        }
    }

    auto packet = socket.listen_packets.top();
    socket.listen_packets.pop();

    logging::logf(logging::log_level::TRACE, "tcp:receive: Received packet\n");

    auto payload_len = tcp_payload_len(packet);

    if(payload_len > n){
        return std::make_unexpected<size_t>(std::ERROR_BUFFER_SMALL);
    }

    auto offset = packet->tag(2) + tcp_data_offset(packet);

    std::copy_n(packet->payload + offset, payload_len, buffer);

    return payload_len;
}

std::expected<network::packet_p> network::tcp::layer::user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor) {
    auto& connection = socket.get_connection_data<tcp_connection>();

    // Make sure stream sockets are connected
    if(!connection.connected){
        return std::make_unexpected<network::packet_p>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    auto target_ip  = connection.server_address;
    auto& interface = network::select_interface(target_ip);

    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{descriptor->payload_size + default_tcp_header_length, target_ip, 0x06};
    auto p = parent->user_prepare_packet(buffer, interface, &desc);

    if (p) {
        auto& packet = *p;

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

    return p;
}

std::expected<void> network::tcp::layer::finalize_packet(network::interface_descriptor& interface, network::socket& socket, network::packet_p& p) {
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(p->payload + p->tag(2));

    auto source_flags = switch_endian_16(tcp_header->flags);

    p->index -= *flag_data_offset(&source_flags) * 4;

    // Compute the checksum
    compute_checksum(*p);

    auto& connection = socket.get_connection_data<tcp_connection>();

    connection.listening = true;

    bool received = false;

    for(size_t t = 0; t < max_tries; ++t){
        logging::logf(logging::log_level::TRACE, "tcp:finalize(std): Send Packet (%h)\n", size_t(source_flags));

        // Give the packet to the IP layer for finalization
        auto result = parent->finalize_packet(interface, p);

        if(!result){
            return result;
        }

        auto before = timer::milliseconds();
        auto after  = before;

        while(true){
            // Make sure we don't wait for more than the timeout
            if (after > before + timeout_ms) {
                break;
            }

            auto remaining = timeout_ms - (after - before);

            // Wait for a packet if necessary
            if(connection.packets.empty()){
                if(!connection.queue.wait_for(remaining)){
                    break;
                }
            }

            thor_assert(!connection.packets.empty(), "Should not be notified if queue not empty");

            auto received_packet = connection.packets.top();
            connection.packets.pop();

            auto* tcp_header = reinterpret_cast<network::tcp::header*>(received_packet->payload + received_packet->tag(2));
            auto flags       = switch_endian_16(tcp_header->flags);

            bool correct_ack = false;
            if(*flag_syn(&source_flags) && *flag_ack(&source_flags)){
                // SYN/ACK should be acknowledge with ACK
                correct_ack = *flag_ack(&flags);
            } else if(*flag_syn(&source_flags)){
                // SYN should be acknowledge with SYN/ACK
                correct_ack = *flag_syn(&flags) && *flag_ack(&flags);
            } else {
                // Other packets should be acknowledge with ACK
                correct_ack = *flag_ack(&flags);
            }

            //TODO Ideally, we should make sure that the ACK is related to
            //the sent packet

            if (correct_ack) {
                logging::logf(logging::log_level::TRACE, "tcp:finalize: Received ACK\n");

                connection.fina_ack_number = switch_endian_32(tcp_header->ack_number);
                connection.fina_seq_number = switch_endian_32(tcp_header->sequence_number);

                received = true;

                break;
            }

            logging::logf(logging::log_level::TRACE, "tcp:finalize: Received unrelated answer\n");
        }

        if(received){
            break;
        }

        after = timer::milliseconds();
    }

    // Stop listening
    connection.listening = false;

    if(received){
        return {};
    } else {
        return std::make_unexpected<void>(std::ERROR_SOCKET_TCP_ERROR);
    }
}

std::expected<size_t> network::tcp::layer::connect(network::socket& sock, network::interface_descriptor& interface, size_t server_port, network::ip::address server) {
    logging::logf(logging::log_level::TRACE, "tcp:connect: Start\n");

    // Create the connection

    auto& connection = connections.create_connection();

    connection.local_port     = ++local_port;
    connection.server_port    = server_port;
    connection.server_address = server;

    // Link the socket and connection
    sock.connection_data = &connection;
    connection.socket = &sock;

    // Prepare the SYN packet

    auto p = kernel_prepare_packet(interface, connection, 0);

    if (!p) {
        return std::make_unexpected<size_t>(p.error());
    }

    auto& packet = *p;

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    auto flags = get_default_flags();
    (flag_syn(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    logging::logf(logging::log_level::TRACE, "tcp:connect: Send SYN\n");

    auto status = finalize_packet(interface, sock, packet);

    if(!status){
        return std::make_unexpected<size_t, size_t>(status.error());
    }

    connection.seq_number = connection.fina_ack_number;
    connection.ack_number = connection.fina_seq_number + 1;

    // The SYN/ACK is ensured by finalize_packet

    logging::logf(logging::log_level::TRACE, "tcp:connect: Received SYN/ACK\n");

    // At this point we have received the SYN/ACK, only remains to ACK

    {
        auto p = kernel_prepare_packet(interface, connection, 0);

        if (!p) {
            return std::make_unexpected<size_t>(p.error());
        }

        auto& packet = *p;

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp:connect: Send ACK\n");

        finalize_packet_direct(interface, packet);
    }

    // Mark the connection as connected

    connection.connected = true;

    logging::logf(logging::log_level::TRACE, "tcp:connect: Done\n");

    return connection.local_port;
}

std::expected<size_t> network::tcp::layer::accept(network::socket& socket){
    auto& connection = socket.get_connection_data<tcp_connection>();

    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    // 1. Wait for SYN

    connection.listening = true;

    logging::logf(logging::log_level::TRACE, "tcp:accept: wait for connection\n");

    uint32_t ack = 0;
    uint32_t seq = 0;

    uint16_t source_port = 0;
    uint16_t target_port = 0;
    uint32_t source_address = 0;

    while (true) {
        if(connection.packets.empty()){
            connection.queue.wait();
        }

        auto received_packet = connection.packets.top();
        connection.packets.pop();

        auto* tcp_header = reinterpret_cast<header*>(received_packet->payload + received_packet->index);
        auto flags       = switch_endian_16(tcp_header->flags);

        if (*flag_syn(&flags)) {
            seq = switch_endian_32(tcp_header->sequence_number);
            ack = switch_endian_32(tcp_header->ack_number);

            source_port = switch_endian_16(tcp_header->source_port);
            target_port = switch_endian_16(tcp_header->target_port);

            auto* ip_header = reinterpret_cast<network::ip::header*>(received_packet->payload + received_packet->tag(1));

            source_address = switch_endian_32(ip_header->source_ip);

            break;
        }
    }

    logging::logf(logging::log_level::TRACE, "tcp:accept: received SYN from %h\n", source_address);

    connection.listening = false;

    // Set the future sequence and acknowledgement numbers
    connection.seq_number = ack;
    connection.ack_number = seq + 1;

    // 2. Prepare the child connection

    auto child_fd = scheduler::register_new_socket(socket.domain, socket.type, socket.protocol);
    auto& child_sock = scheduler::get_socket(child_fd);

    logging::logf(logging::log_level::TRACE, "tcp:accept: Register new socket %u\n", child_fd);

    // Create the connection

    auto& child_connection = connections.create_connection();

    child_connection.child = true;

    child_connection.local_port     = target_port;
    child_connection.server_port    = source_port;
    child_connection.server_address = source_address;

    // Link the socket and connection
    child_sock.connection_data = &child_connection;
    child_connection.socket = &child_sock;

    // Child connection numbers
    child_connection.seq_number = connection.seq_number;
    child_connection.ack_number = connection.ack_number;

    child_connection.connected = true;

    auto& interface = network::select_interface(source_address);

    // 3. Send SYN/ACK

    {
        auto p = kernel_prepare_packet(interface, child_connection, 0);

        if (!p) {
            return std::make_unexpected<size_t>(p.error());
        }

        auto& packet = *p;

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        (flag_syn(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp:accept: Send SYN/ACK %h\n", size_t(flags));

        auto status = finalize_packet(interface, child_sock, packet);

        if(!status){
            return std::make_unexpected<size_t, size_t>(status.error());
        }
    }

    // The ACK is enforced by finalize_packet

    logging::logf(logging::log_level::TRACE, "tcp:accept: Done\n");

    return {child_fd};
}

std::expected<size_t> network::tcp::layer::accept(network::socket& socket, size_t ms){
    auto& connection = socket.get_connection_data<tcp_connection>();

    if(!connection.connected){
        return std::make_unexpected<size_t>(std::ERROR_SOCKET_NOT_CONNECTED);
    }

    // 1. Wait for SYN

    connection.listening = true;

    logging::logf(logging::log_level::TRACE, "tcp:accept: wait for connection\n");

    uint32_t ack = 0;
    uint32_t seq = 0;

    uint16_t source_port = 0;
    uint16_t target_port = 0;
    uint32_t source_address = 0;

    auto before = timer::milliseconds();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + ms) {
            return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
        }

        auto remaining = ms - (after - before);

        if (connection.packets.empty()) {
            if (!connection.queue.wait_for(remaining)) {
                return std::make_unexpected<size_t>(std::ERROR_SOCKET_TIMEOUT);
            }
        }

        auto received_packet = connection.packets.top();
        connection.packets.pop();

        auto* tcp_header = reinterpret_cast<header*>(received_packet->payload + received_packet->index);
        auto flags       = switch_endian_16(tcp_header->flags);

        if (*flag_syn(&flags)) {
            seq = switch_endian_32(tcp_header->sequence_number);
            ack = switch_endian_32(tcp_header->ack_number);

            source_port = switch_endian_16(tcp_header->source_port);
            target_port = switch_endian_16(tcp_header->target_port);

            auto* ip_header = reinterpret_cast<network::ip::header*>(received_packet->payload + received_packet->tag(1));

            source_address = switch_endian_32(ip_header->source_ip);

            break;
        }

        after = timer::milliseconds();
    }

    logging::logf(logging::log_level::TRACE, "tcp:accept: received SYN from %h\n", source_address);

    connection.listening = false;

    // Set the future sequence and acknowledgement numbers
    connection.seq_number = ack;
    connection.ack_number = seq + 1;

    // 2. Prepare the child connection

    auto child_fd = scheduler::register_new_socket(socket.domain, socket.type, socket.protocol);
    auto& child_sock = scheduler::get_socket(child_fd);

    logging::logf(logging::log_level::TRACE, "tcp:accept: Register new socket %u\n", child_fd);

    // Create the connection

    auto& child_connection = connections.create_connection();

    child_connection.child = true;

    child_connection.local_port     = target_port;
    child_connection.server_port    = source_port;
    child_connection.server_address = source_address;

    // Link the socket and connection
    child_sock.connection_data = &child_connection;
    child_connection.socket = &child_sock;

    // Child connection numbers
    child_connection.seq_number = connection.seq_number;
    child_connection.ack_number = connection.ack_number;

    child_connection.connected = true;

    auto& interface = network::select_interface(source_address);

    // 3. Send SYN/ACK

    {
        auto p = kernel_prepare_packet(interface, child_connection, 0);

        if (!p) {
            return std::make_unexpected<size_t>(p.error());
        }

        auto& packet = *p;

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        (flag_syn(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp:accept: Send SYN/ACK %h\n", size_t(flags));

        auto status = finalize_packet(interface, child_sock, packet);

        if(!status){
            return std::make_unexpected<size_t, size_t>(status.error());
        }
    }

    // The ACK is enforced by finalize_packet

    logging::logf(logging::log_level::TRACE, "tcp:accept: Done\n");

    return {child_fd};
}

std::expected<void> network::tcp::layer::server_start(network::socket& sock, size_t server_port, network::ip::address server) {
    // Create the connection

    auto& connection = connections.create_connection();

    connection.server         = true;
    connection.server_port    = server_port;
    connection.server_address = server;

    // Link the socket and connection
    sock.connection_data = &connection;
    connection.socket = &sock;

    // Mark the connection as connected

    connection.connected = true;

    return {};
}

std::expected<void> network::tcp::layer::disconnect(network::socket& sock) {
    logging::logf(logging::log_level::TRACE, "tcp:disconnect: Disconnect\n");

    auto& connection = sock.get_connection_data<tcp_connection>();

    if(!connection.connected){
        return std::make_expected();
    }

    auto target_ip  = connection.server_address;
    auto& interface = network::select_interface(target_ip);

    auto p = kernel_prepare_packet(interface, connection, 0);

    if (!p) {
        return std::make_unexpected<void>(p.error());
    }

    auto& packet = *p;

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    auto flags = get_default_flags();
    (flag_fin(&flags)) = 1;
    (flag_ack(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    connection.listening = true;

    logging::logf(logging::log_level::TRACE, "tcp:disconnect: Send FIN/ACK\n");

    bool rec_fin_ack = false;
    bool rec_ack     = false;

    uint32_t seq = 0;
    uint32_t ack = 0;

    bool received = false;

    for(size_t t = 0; t < max_tries; ++t){
        // Give the packet to the IP layer for finalization
        auto result = finalize_packet_direct(interface, packet);

        if(!result){
            return result;
        }

        auto before = timer::milliseconds();
        auto after  = before;

        while(true){
            // Make sure we don't wait for more than the timeout
            if (after > before + timeout_ms) {
                break;
            }

            auto remaining = timeout_ms - (after - before);

            if(connection.queue.wait_for(remaining)){
                auto received_packet = connection.packets.top();
                connection.packets.pop();

                auto* tcp_header = reinterpret_cast<header*>(received_packet->payload + received_packet->index);
                auto flags       = switch_endian_16(tcp_header->flags);

                bool correct_ack = false;
                if (*flag_fin(&flags) && *flag_ack(&flags)) {
                    rec_fin_ack     = true;
                    correct_ack = true;
                } else if (*flag_ack(&flags)) {
                    rec_ack         = true;
                    correct_ack = true;
                }

                if (correct_ack) {
                    seq = switch_endian_32(tcp_header->sequence_number);
                    ack = switch_endian_32(tcp_header->ack_number);

                    received = true;

                    break;
                }
            } else {
                break;
            }
        }

        if(received){
            break;
        }

        after = timer::milliseconds();
    }

    if(!received){
        return std::make_unexpected<void>(std::ERROR_SOCKET_TCP_ERROR);
    }

    // Set the future sequence and acknowledgement numbers
    connection.seq_number = ack;
    connection.ack_number = seq + 1;

    // If we received an ACK, we must wait for a FIN/ACK from the server now
    if(rec_ack){
        logging::logf(logging::log_level::TRACE, "tcp:disconnect: Received ACK waiting for FIN/ACK\n");

        received = false;

        auto before = timer::milliseconds();
        auto after  = before;

        while(true){
            // Make sure we don't wait for more than the timeout
            if (after > before + timeout_ms) {
                break;
            }

            auto remaining = timeout_ms - (after - before);

            if(connection.packets.empty()){
                if(!connection.queue.wait_for(remaining)){
                    break;
                }
            }

            auto received_packet = connection.packets.top();
            connection.packets.pop();

            auto* tcp_header = reinterpret_cast<header*>(received_packet->payload + received_packet->index);
            auto flags       = switch_endian_16(tcp_header->flags);

            bool correct_ack = *flag_fin(&flags) && *flag_ack(&flags);

            if (correct_ack) {
                seq = switch_endian_32(tcp_header->sequence_number);
                ack = switch_endian_32(tcp_header->ack_number);

                received = true;

                break;
            }
        }

        if(!received){
            return std::make_unexpected<void>(std::ERROR_SOCKET_TCP_ERROR);
        }

        // Set the future sequence and acknowledgement numbers
        connection.seq_number = ack;
        connection.ack_number = seq + 1;

        logging::logf(logging::log_level::TRACE, "tcp:disconnect: Received FIN/ACK waiting for ACK\n");
    } else if(rec_fin_ack) {
        logging::logf(logging::log_level::TRACE, "tcp:disconnect: Received FIN/ACK directly waiting for ACK\n");
    }

    // Stop listening
    connection.listening = false;

    // Finally we send the ACK for the FIN/ACK

    {
        auto p = kernel_prepare_packet(interface, connection, 0);

        if (!p) {
            return std::make_unexpected<void>(p.error());
        }

        auto& packet = *p;

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp: Send ACK\n");
        finalize_packet_direct(interface, packet);
    }

    // Mark the connection as connected

    connection.connected = false;

    connections.remove_connection(connection);

    return {};
}

// This is used for raw answer
std::expected<network::packet_p> network::tcp::layer::kernel_prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size) {
    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{payload_size + default_tcp_header_length, target_ip, 0x06};
    auto packet = parent->kernel_prepare_packet(interface, desc);

    if (packet) {
        ::prepare_packet(**packet, source, target);
    }

    return packet;
}

// This is used for TCP connection answers
std::expected<network::packet_p> network::tcp::layer::kernel_prepare_packet(network::interface_descriptor& interface, tcp_connection& connection, size_t payload_size) {
    auto target_ip   = connection.server_address;
    auto local_port  = connection.local_port;
    auto server_port = connection.server_port;

    // Ask the IP layer to craft a packet
    network::ip::packet_descriptor desc{payload_size + default_tcp_header_length, target_ip, 0x06};
    auto p = parent->kernel_prepare_packet(interface, desc);

    if (p) {
        auto& packet = *p;

        ::prepare_packet(*packet, local_port, server_port);

        auto* tcp_header = reinterpret_cast<network::tcp::header*>(packet->payload + packet->tag(2));

        tcp_header->sequence_number = switch_endian_32(connection.seq_number);
        tcp_header->ack_number      = switch_endian_32(connection.ack_number);
    }

    return p;
}

// finalize without waiting for ACK
std::expected<void> network::tcp::layer::finalize_packet_direct(network::interface_descriptor& interface, network::packet_p& p) {
    auto* tcp_header = reinterpret_cast<network::tcp::header*>(p->payload + p->tag(2));

    auto flags = switch_endian_16(tcp_header->flags);

    p->index -= *flag_data_offset(&flags) * 4;

    // Compute the checksum
    compute_checksum(*p);

    // Give the packet to the IP layer for finalization
    return parent->finalize_packet(interface, p);
}
