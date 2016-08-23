//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <types.hpp>

namespace network {

enum class socket_domain {
    AF_INET
};

enum class socket_type {
    RAW
};

enum class socket_protocol {
    ICMP
};

struct socket {
    size_t id;
    socket_domain domain;
    socket_type type;
    socket_protocol protocol;

    //socket(){}
    //socket(size_t id, socket_domain domain, socket_type type, socket_protocol protocol)
            //: id(id), domain(domain), type(type), protocol(protocol) {}

    void invalidate(){
        id = 0xFFFFFFFF;
    }

    bool is_valid() const {
        return id == 0xFFFFFFFF;
    }
};

} // end of network namespace

#endif
