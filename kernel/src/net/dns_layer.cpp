//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <bit_field.hpp>

#include "net/dns_layer.hpp"
#include "net/udp_layer.hpp"
#include "net/ip_layer.hpp"

#include "kernel_utils.hpp"

#include "tlib/errors.hpp"

namespace {

using flag_qr     = std::bit_field<uint16_t, uint8_t, 15, 1>;
using flag_opcode = std::bit_field<uint16_t, uint8_t, 11, 4>;
using flag_aa     = std::bit_field<uint16_t, uint8_t, 10, 1>;
using flag_tc     = std::bit_field<uint16_t, uint8_t, 9, 1>;
using flag_rd     = std::bit_field<uint16_t, uint8_t, 8, 1>;
using flag_ra     = std::bit_field<uint16_t, uint8_t, 7, 1>;
using flag_zeroes = std::bit_field<uint16_t, uint8_t, 4, 3>;
using flag_rcode  = std::bit_field<uint16_t, uint8_t, 0, 4>;

void prepare_packet_query(network::packet& packet, uint16_t identification) {
    packet.tag(3, packet.index);

    // Set the DNS header

    auto* dns_header = reinterpret_cast<network::dns::header*>(packet.payload + packet.index);

    // Set the identification
    dns_header->identification = switch_endian_16(identification);

    // There is one question, nothing else
    dns_header->questions      = switch_endian_16(1);
    dns_header->answers        = switch_endian_16(0);
    dns_header->authority_rrs  = switch_endian_16(0);
    dns_header->additional_rrs = switch_endian_16(0);

    uint16_t flags = 0;

    // Set all the flags
    (flag_qr(&flags)) = 0;     // This is a query
    (flag_opcode(&flags)) = 0; // This is a standard query
    (flag_aa(&flags)) = 0;     // This is a query (field not used)
    (flag_tc(&flags)) = 0;     // The question is not truncated
    (flag_rd(&flags)) = 1;     // No need for recursion
    (flag_ra(&flags)) = 0;     // This is a query (field not used)
    (flag_zeroes(&flags)) = 0; // Always zero
    (flag_rcode(&flags)) = 0;  // This is a query (field not used)

    dns_header->flags = switch_endian_16(flags);

    packet.index += sizeof(network::dns::header);
}

std::string decode_domain(char* payload, size_t& offset) {
    std::string domain;

    offset = 0;

    while (true) {
        auto label_size = static_cast<uint8_t>(*(payload + offset));
        ++offset;

        if (!label_size) {
            break;
        }

        if (!domain.empty()) {
            domain += '.';
        }

        for (size_t i = 0; i < label_size; ++i) {
            domain += *(payload + offset);
            ++offset;
        }
    }

    return domain;
}

} //end of anonymous namespace

network::dns::layer::layer(network::udp::layer* parent) : parent(parent) {
    parent->register_dns_layer(this);
}

void network::dns::layer::decode(network::interface_descriptor& /*interface*/, network::packet_p& packet) {
    packet->tag(3, packet->index);

    auto* dns_header = reinterpret_cast<header*>(packet->payload + packet->index);

    logging::logf(logging::log_level::TRACE, "dns: Start DNS packet handling\n");

    auto identification = switch_endian_16(dns_header->identification);
    auto questions      = switch_endian_16(dns_header->questions);
    auto answers        = switch_endian_16(dns_header->answers);
    auto authority_rrs  = switch_endian_16(dns_header->authority_rrs);
    auto additional_rrs = switch_endian_16(dns_header->additional_rrs);

    logging::logf(logging::log_level::TRACE, "dns: Identification %h \n", size_t(identification));
    logging::logf(logging::log_level::TRACE, "dns: Answers %u \n", size_t(answers));
    logging::logf(logging::log_level::TRACE, "dns: Questions %u \n", size_t(questions));
    logging::logf(logging::log_level::TRACE, "dns: Authorithy RRs %u \n", size_t(authority_rrs));
    logging::logf(logging::log_level::TRACE, "dns: Additional RRs %u \n", size_t(additional_rrs));

    auto flags = switch_endian_16(dns_header->flags);

    if (*flag_qr(&flags) == 0) {
        logging::logf(logging::log_level::TRACE, "dns: Query\n");
    } else {
        auto response_code = *flag_opcode(&flags);

        if (response_code == 0x0) {
            logging::logf(logging::log_level::TRACE, "dns: Response OK\n");

            auto* payload = packet->payload + packet->index + sizeof(header);

            // Decode the questions (simply wrap around it)

            for (size_t i = 0; i < questions; ++i) {
                size_t length;
                auto domain = decode_domain(payload, length);

                payload += length;

                auto rr_type = switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                payload += 2;

                auto rr_class = switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                payload += 2;

                logging::logf(logging::log_level::TRACE, "dns: Query %u Type %u Class %u Name %s\n", i, rr_type, rr_class, domain.c_str());
            }

            for (size_t i = 0; i < answers; ++i) {
                auto label = static_cast<uint8_t>(*payload);

                std::string domain;
                if (label > 64) {
                    // This is a pointer
                    auto pointer = switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                    auto offset  = pointer & (0xFFFF >> 2);

                    payload += 2;

                    size_t ignored;
                    domain = decode_domain(packet->payload + packet->index + offset, ignored);
                } else {
                    logging::logf(logging::log_level::TRACE, "dns: Unable to handle non-compressed data\n");
                    return;
                }

                auto rr_type = switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                payload += 2;

                auto rr_class = switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                payload += 2;

                auto ttl = switch_endian_32(*reinterpret_cast<uint32_t*>(payload));
                payload += 4;

                auto rd_length = switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                payload += 2;

                if (rr_type == 0x1 && rr_class == 0x1) {
                    auto ip     = network::ip::ip32_to_ip(*reinterpret_cast<uint32_t*>(payload));
                    auto ip_str = network::ip::ip_to_str(ip);

                    logging::logf(logging::log_level::TRACE, "dns: Answer %u Domain %s Type %u Class %u TTL %u IP: %s\n", i, domain.c_str(), rr_type, rr_class, ttl, ip_str.c_str());
                } else {
                    logging::logf(logging::log_level::TRACE, "dns: Answer %u Domain %s Type %u Class %u TTL %u \n", i, rr_type, rr_class, ttl, domain.c_str());
                    logging::logf(logging::log_level::TRACE, "dns: Answer %u Unable to read data for type and class\n", i);
                }

                payload += rd_length;
            }
        } else if (response_code == 0x1) {
            logging::logf(logging::log_level::TRACE, "dns: Format Error\n");
        } else if (response_code == 0x2) {
            logging::logf(logging::log_level::TRACE, "dns: Server Failure\n");
        } else if (response_code == 0x3) {
            logging::logf(logging::log_level::TRACE, "dns: Name Error\n");
        } else if (response_code == 0x4) {
            logging::logf(logging::log_level::TRACE, "dns: Not Implemented\n");
        } else if (response_code == 0x5) {
            logging::logf(logging::log_level::TRACE, "dns: Refused\n");
        }
    }

    // Note: Propagate is handled by UDP connections
}

std::expected<network::packet_p> network::dns::layer::user_prepare_packet(char* buffer, network::socket& socket, const packet_descriptor* descriptor) {
    // Check the packet descriptor
    if(!descriptor->query){
        return std::make_unexpected<network::packet_p>(std::ERROR_SOCKET_INVALID_PACKET_DESCRIPTOR);
    }

    // Ask the UDP layer to craft a packet
    network::udp::packet_descriptor desc{sizeof(header) + descriptor->payload_size};
    auto packet = parent->user_prepare_packet(buffer, socket, &desc);

    if (packet) {
        ::prepare_packet_query(**packet, descriptor->identification);
    }

    return packet;
}

std::expected<void> network::dns::layer::finalize_packet(network::interface_descriptor& interface, network::packet_p& p) {
    p->index -= sizeof(header);

    // Give the packet to the UDP layer for finalization
    return parent->finalize_packet(interface, p);
}

std::expected<void> network::dns::layer::finalize_packet(network::interface_descriptor& interface, network::socket& /*sock*/, network::packet_p& p) {
    return this->finalize_packet(interface, p);
}
