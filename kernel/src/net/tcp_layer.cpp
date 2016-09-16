//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>
#include <atomic.hpp>
#include <list.hpp>
#include <sleep_queue.hpp>

#include "net/tcp_layer.hpp"
#include "net/dns_layer.hpp"
#include "net/checksum.hpp"

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

struct tcp_listener {
    size_t source_port;
    size_t target_port;
    std::atomic<bool> active;
    sleep_queue queue;
    circular_buffer<network::ethernet::packet, 8> packets;

    tcp_listener(size_t source_port, size_t target_port)
            : source_port(source_port), target_port(target_port), active(false) {
        //Nothing else to init
    }
};

// Note: We need a list to not invalidate the values during insertions
std::list<tcp_listener> listeners;

tcp_listener* get_listener(size_t source_port, size_t target_port) {
    auto end = listeners.end();
    auto it  = listeners.begin();

    while (it != end) {
        auto& listener = *it;

        if (listener.source_port == source_port && listener.target_port == target_port) {
            return &listener;
        }

        ++it;
    }

    return nullptr;
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

} //end of anonymous namespace

void network::tcp::decode(network::interface_descriptor& /*interface*/, network::ethernet::packet& packet) {
    packet.tag(2, packet.index);

    auto* tcp_header = reinterpret_cast<header*>(packet.payload + packet.index);

    logging::logf(logging::log_level::TRACE, "tcp: Start TCP packet handling\n");

    auto source_port = switch_endian_16(tcp_header->source_port);
    auto target_port = switch_endian_16(tcp_header->target_port);
    auto sequence    = switch_endian_32(tcp_header->sequence_number);
    auto ack         = switch_endian_32(tcp_header->ack_number);

    logging::logf(logging::log_level::TRACE, "tcp: Source Port %u \n", size_t(source_port));
    logging::logf(logging::log_level::TRACE, "tcp: Target Port %u \n", size_t(target_port));
    logging::logf(logging::log_level::TRACE, "tcp: Seq Number %u \n", size_t(sequence));
    logging::logf(logging::log_level::TRACE, "tcp: Ack Number %u \n", size_t(ack));

    // Propagate to kernel listeners

    auto end = listeners.end();
    auto it  = listeners.begin();

    while (it != end) {
        auto& listener = *it;

        if (listener.active.load() && listener.source_port == source_port && listener.target_port == target_port) {
            auto copy    = packet;
            copy.payload = new char[copy.payload_size];
            std::copy_n(packet.payload, packet.payload_size, copy.payload);

            listener.packets.push(copy);
            listener.queue.wake_up();
        }

        ++it;
    }

    packet.index += sizeof(header);

    network::propagate_packet(packet, network::socket_protocol::TCP);
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(network::interface_descriptor& interface, network::ip::address target_ip, size_t source, size_t target, size_t payload_size) {
    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(interface, sizeof(header) + payload_size, target_ip, 0x06);

    if (packet) {
        ::prepare_packet(*packet, source, target);
    }

    return packet;
}

std::expected<network::ethernet::packet> network::tcp::prepare_packet(char* buffer, network::interface_descriptor& interface, network::socket& socket, size_t payload_size) {
    auto target_ip = socket.server_address;

    // Ask the IP layer to craft a packet
    auto packet = network::ip::prepare_packet(buffer, interface, sizeof(header) + payload_size, target_ip, 0x06);

    if (packet) {
        auto source = socket.local_port;
        auto target = socket.server_port;

        ::prepare_packet(*packet, source, target);

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        auto flags = get_default_flags();
        (flag_psh(&flags)) = 1;
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        tcp_header->sequence_number = switch_endian_32(socket.seq_number);
        tcp_header->ack_number      = switch_endian_32(socket.ack_number);
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

    auto source = socket.local_port;
    auto target = socket.server_port;

    auto listener_ptr = get_listener(target, source);

    if (!listener_ptr) {
        logging::logf(logging::log_level::ERROR, "tcp: Unable to find listener!\n");
        return; //TODO Fail
    }

    auto& listener = *listener_ptr;

    listener.active = true;

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

            if(listener.queue.sleep(remaining)){
                auto received_packet = listener.packets.pop();

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
    listener.active = false;

    if(received){
        // Set the future sequence and acknowledgement numbers
        socket.seq_number = ack;
        socket.ack_number = seq;
    } else {
        //TODO We need to be able to make finalize fail!
    }
}

std::expected<void> network::tcp::connect(network::socket& sock, network::interface_descriptor& interface) {
    auto target_ip = sock.server_address;
    auto source    = sock.local_port;
    auto target    = sock.server_port;

    sock.seq_number = 0;
    sock.ack_number = 0;

    auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

    if (!packet) {
        return std::make_unexpected<void>(packet.error());
    }

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    tcp_header->sequence_number = 0;
    tcp_header->ack_number      = 0;

    auto flags = get_default_flags();
    (flag_syn(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    // Create the listener

    auto& listener = listeners.emplace_back(target, source);

    listener.active = true;

    logging::logf(logging::log_level::TRACE, "tcp: Send SYN\n");
    tcp::finalize_packet(interface, *packet);

    uint32_t seq;
    uint32_t ack;

    while (true) {
        // TODO Need a timeout
        listener.queue.sleep();
        auto received_packet = listener.packets.pop();

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

    listener.active = false;

    sock.seq_number = ack;
    sock.ack_number = seq + 1;

    // At this point we have received the SYN/ACK, only remains to ACK

    {
        auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

        if (!packet) {
            return std::make_unexpected<void>(packet.error());
        }

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        tcp_header->sequence_number = switch_endian_32(sock.seq_number);
        tcp_header->ack_number      = switch_endian_32(sock.ack_number);

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp: Send ACK\n");
        tcp::finalize_packet(interface, *packet);
    }

    return {};
}

std::expected<void> network::tcp::disconnect(network::socket& sock, network::interface_descriptor& interface) {
    auto target_ip = sock.server_address;
    auto source    = sock.local_port;
    auto target    = sock.server_port;

    auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

    if (!packet) {
        return std::make_unexpected<void>(packet.error());
    }

    auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

    tcp_header->sequence_number = switch_endian_32(sock.seq_number);
    tcp_header->ack_number      = switch_endian_32(sock.ack_number);

    auto flags = get_default_flags();
    (flag_fin(&flags)) = 1;
    (flag_ack(&flags)) = 1;
    tcp_header->flags = switch_endian_16(flags);

    auto& listener = listeners.emplace_back(target, source);

    listener.active = true;

    logging::logf(logging::log_level::TRACE, "tcp: Send FIN/ACK\n");
    tcp::finalize_packet(interface, *packet);

    uint32_t seq;
    uint32_t ack;

    auto fin_rec     = false;
    auto fin_ack_rec = false;

    while (true) {
        // TODO Need a timeout
        listener.queue.sleep();
        auto received_packet = listener.packets.pop();

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

    listener.active = false;

    sock.seq_number = ack;
    sock.ack_number = seq + 1;

    // At this point we have received the FIN/ACK, only remains to ACK

    {
        auto packet = tcp::prepare_packet(interface, target_ip, source, target, 0);

        if (!packet) {
            return std::make_unexpected<void>(packet.error());
        }

        auto* tcp_header = reinterpret_cast<header*>(packet->payload + packet->tag(2));

        tcp_header->sequence_number = switch_endian_32(sock.seq_number);
        tcp_header->ack_number      = switch_endian_32(sock.ack_number);

        auto flags = get_default_flags();
        (flag_ack(&flags)) = 1;
        tcp_header->flags = switch_endian_16(flags);

        logging::logf(logging::log_level::TRACE, "tcp: Send ACK\n");
        tcp::finalize_packet(interface, *packet);
    }

    auto end = listeners.end();
    auto it  = listeners.begin();

    while (it != end) {
        if (&(*it) == &listener) {
            listeners.erase(it);
            break;
        }

        ++it;
    }

    return {};
}
