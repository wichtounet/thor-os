//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT) //=======================================================================

#ifndef TLIB_NET_CONSTANTS_H
#define TLIB_NET_CONSTANTS_H

#include <types.hpp>

#include "tlib/config.hpp"

THOR_NAMESPACE(tlib, network) {

namespace ip {

/*!
 * \brief An IP Addres helper
 */
struct address {
    uint32_t raw_address = 0;

    address(){}
    address(uint32_t raw) : raw_address(raw) {}

    uint8_t operator()(size_t index) const {
        return (raw_address >> ((3 - index) * 8)) & 0xFF;
    }

    void set_sub(size_t index, uint8_t value){
        raw_address |= uint32_t(value) << ((3 - index) * 8);
    }

    bool operator==(const address& rhs) const {
        return this->raw_address == rhs.raw_address;
    }
};

inline address make_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
    address addr;
    addr.set_sub(0, a);
    addr.set_sub(1, b);
    addr.set_sub(2, c);
    addr.set_sub(3, d);
    return addr;
}

/*!
 * \brief The header of an IP packet
 */
struct header {
    uint8_t version_ihl; ///< The version and header length
    uint8_t dscp_ecn;
    uint16_t total_len; ///< The total length of the packet
    uint16_t identification; ///< The identification of the packet
    uint16_t flags_offset; ///< The flags and data offset
    uint8_t ttl; ///< The time to live of the packet
    uint8_t protocol; ///< The protocol used
    uint16_t header_checksum; ///< The checksum
    uint32_t source_ip; ///< The source IP address
    uint32_t target_ip; ///< The target IP address
} __attribute__((packed));

struct packet_descriptor {
    size_t size;
    address destination;
    size_t protocol;
};

} // end of namespace ip

struct inet_address {
    ip::address address;
    size_t port;
};

namespace ethernet {

struct address {
    char mac[6];
} __attribute__((packed));

/*!
 * \brief The header of an ethernet packet
 */
struct header {
    address target; ///< The target MAC address
    address source; ///< The source MAC address
    uint16_t type; ///< The type of packet (the child layer)
} __attribute__((packed));

enum class ether_type {
    IPV4,
    IPV6,
    ARP,
    UNKNOWN
};

struct packet_descriptor {
    size_t size;
    size_t destination;
    ether_type type;
};

} // end of namespace ethernet

namespace icmp {

enum class type : uint8_t {
    ECHO_REPLY = 0,
    UNREACHABLE = 3,
    SOURCE_QUENCH = 4,
    REDICT = 5,
    ECHO_REQUEST = 8,
    ROUTER_ADVERTISEMENT = 9,
    ROUTER_SOLICITATION = 10,
    TIME_EXCEEDED = 11,
    PARAMETER_PROBLEM = 12,
    TIMESTAMP = 13,
    TIMESTAMP_REPLY = 14,
    INFORMATION_REQUEST = 15,
    INFORMATION_REPLY = 16,
    ADDRESS_MASK_REQUEST = 17,
    ADDRESS_MASK_REPLY = 18,
    TRACEROUTE = 30
};

/*!
 * \brief The header of an ICMP packet
 */
struct header {
    uint8_t type; ///< The type of message
    uint8_t code; ///< The code of the transmission (sub-type)
    uint16_t checksum; ///< The checksum
    uint32_t rest; ///< Depends on the type of packet type
} __attribute__((packed));

/*!
 * \brief The rest header of an ICMP request packet
 */
struct echo_request_header {
    uint16_t identifier; ///< The identifier
    uint16_t sequence;   ///< The sequence number
} __attribute__((packed));

struct packet_descriptor {
    size_t payload_size;
    ip::address target_ip;
    icmp::type type;
    size_t code;
};

} // end of namespace icmp

namespace http {

struct packet_descriptor {
    size_t payload_size;
    ip::address target_ip;
};

} // end of http namespace

namespace dns {

/*!
 * \brief The header of a DNS packet
 */
struct header {
    uint16_t identification; ///< The identification of the query
    uint16_t flags; ///< The flags
    uint16_t questions; ///< The number of questions
    uint16_t answers; ///< The number of answers
    uint16_t authority_rrs; ///< The number of authorithy Records
    uint16_t additional_rrs; ///< The number of additional Records
} __attribute__((packed));

struct packet_descriptor {
    size_t payload_size;
    uint16_t identification;
    bool query;
};

} // end of dns namespace

namespace udp {

struct packet_descriptor {
    size_t payload_size;
};

} // end of udp namespace

namespace tcp {

/*!
 * \brief The header of a TCP packet
 */
struct header {
    uint16_t source_port; ///< The TCP source port
    uint16_t target_port; ///< The TCP target port
    uint32_t sequence_number; ///< The sequence number
    uint32_t ack_number; ///< The acknowledge number
    uint16_t flags; ///< The flags
    uint16_t window_size; ///< The size of the receiving window
    uint16_t checksum; ///< The checksum
    uint16_t urgent_pointer; ///< Indicates if the packet is urgent
} __attribute__((packed));

struct packet_descriptor {
    size_t payload_size;
};

} // end of tcp namespace

enum class socket_domain : size_t {
    AF_INET
};

enum class socket_type : size_t {
    RAW,
    DGRAM,
    STREAM
};

enum class socket_protocol : size_t {
    ICMP,
    DNS,
    TCP,
    UDP
};

} // end of network namespace

#endif
