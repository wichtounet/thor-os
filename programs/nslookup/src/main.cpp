//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>
#include <tlib/net.hpp>
#include <tlib/dns.hpp>

namespace {

static constexpr const size_t retries    = 10;
static constexpr const size_t timeout_ms = 2000;

bool send_request(tlib::socket& sock, const std::string& domain){
    auto parts = std::split(domain, '.');

    size_t characters = domain.size() - (parts.size() - 1); // The dots are not included
    size_t labels     = parts.size();

    tlib::dns::packet_descriptor desc;
    desc.payload_size   = labels + characters + 1 + 2 * 2;
    desc.target_ip      = tlib::ip::make_address(10, 0, 2, 3);
    desc.source_port    = 3456;
    desc.identification = 0x666;
    desc.query          = true;

    auto packet = sock.prepare_packet(&desc);

    if (!sock) {
        tlib::printf("nslookup: prepare_packet error: %s\n", std::error_message(sock.error()));
        return false;
    }

    auto* payload = reinterpret_cast<char*>(packet.payload + packet.index);

    size_t i = 0;
    for (auto& part : parts) {
        payload[i++] = part.size();

        for (size_t j = 0; j < part.size(); ++j) {
            payload[i++] = part[j];
        }
    }

    payload[i++] = 0;

    auto* q_type = reinterpret_cast<uint16_t*>(packet.payload + packet.index + i);
    *q_type      = 0x0100; // A Record

    auto* q_class = reinterpret_cast<uint16_t*>(packet.payload + packet.index + i + 2);
    *q_class      = 0x0100; // IN (internet)

    sock.finalize_packet(packet);

    if (!sock) {
        tlib::printf("nslookup: finalize_packet error: %s\n", std::error_message(sock.error()));
        return false;
    }

    return true;
}

} // end of anonymous namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        tlib::print_line("usage: nslookup domain");
        return 1;
    }

    std::string domain(argv[1]);

    tlib::socket sock(tlib::socket_domain::AF_INET, tlib::socket_type::RAW, tlib::socket_protocol::DNS);

    if (!sock) {
        tlib::printf("nslookup: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    sock.listen(true);

    if (!sock) {
        tlib::printf("nslookup: socket error: %s\n", std::error_message(sock.error()));
        return 1;
    }

    size_t tries = 0;

    if(!send_request(sock, domain)){
        return 1;
    }

    auto before = tlib::ms_time();
    auto after  = before;

    while (true) {
        // Make sure we don't wait for more than the timeout
        if (after > before + timeout_ms) {
            break;
        }

        auto remaining = timeout_ms - (after - before);

        auto p = sock.wait_for_packet(remaining);
        if (!sock) {
            tlib::printf("nslookup: wait_for_packet error: %s\n", std::error_message(sock.error()));
            return 1;
        } else {
            auto* dns_header = reinterpret_cast<tlib::dns::header*>(p.payload + p.index);

            auto identification = tlib::switch_endian_16(dns_header->identification);

            // Only handle packet with the correct identification
            if (identification == 0x666) {
                auto questions = tlib::switch_endian_16(dns_header->questions);
                auto answers   = tlib::switch_endian_16(dns_header->answers);

                auto flags = dns_header->flags;
                auto qr    = flags >> 15;

                // Only handle Response
                if (qr) {
                    auto rcode = flags & 0xF;

                    if (rcode == 0x0 && answers > 0) {
                        auto* payload = p.payload + p.index + sizeof(tlib::dns::header);

                        // Decode the questions (simply wrap around it)

                        for (size_t i = 0; i < questions; ++i) {
                            size_t length;
                            auto domain = tlib::dns::decode_domain(payload, length);

                            payload += length;
                            payload += 4;

                            tlib::printf("DNS Question %s\n", domain.c_str());
                        }

                        tlib::printf("DNS Answers:\n");

                        for (size_t i = 0; i < answers; ++i) {
                            auto label = static_cast<uint8_t>(*payload);

                            std::string domain;
                            if (label > 64) {
                                // This is a pointer
                                auto pointer = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                                auto offset  = pointer & (0xFFFF >> 2);

                                payload += 2;

                                size_t ignored;
                                domain = tlib::dns::decode_domain(p.payload + p.index + offset, ignored);
                            } else {
                                tlib::printf("nslookup: Cannot read DNS response\n");
                                return 1;
                            }

                            auto rr_type = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                            payload += 2;

                            auto rr_class = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                            payload += 2;

                            auto ttl = tlib::switch_endian_32(*reinterpret_cast<uint32_t*>(payload));
                            payload += 4;

                            auto rd_length = tlib::switch_endian_16(*reinterpret_cast<uint16_t*>(payload));
                            payload += 2;

                            if (rr_type == 0x1 && rr_class == 0x1) {
                                auto ip = reinterpret_cast<uint8_t*>(payload);

                                tlib::printf("  Answer %u Domain %s Type %u Class %u TTL %u IP: %u.%u.%u.%u\n",
                                             i, domain.c_str(), rr_type, rr_class, ttl, ip[3], ip[2], ip[1], ip[0]);
                            } else {
                                tlib::printf("nslookup: Cannot read DNS response\n");
                                return 1;
                            }

                            payload += rd_length;
                        }

                        break;
                    } else {
                        // There was an error, retry
                        if(++tries == retries || !send_request(sock, domain)){
                            return 1;
                        }
                    }
                }
            }
        }

        after = tlib::ms_time();
    }

    sock.listen(false);

    return 0;
}
