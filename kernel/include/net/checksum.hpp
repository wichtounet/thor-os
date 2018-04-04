//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef NET_CHECKSUM_H
#define NET_CHECKSUM_H

#include <types.hpp>

namespace network {

template<typename T>
uint32_t checksum_add_bytes(T* values, size_t length){
    auto raw_values = reinterpret_cast<uint8_t*>(values);

    uint32_t sum = 0;

    for(size_t i = 0; i < length; ++i){
        if(i & 1){
            sum += static_cast<uint32_t>(raw_values[i]);
        } else {
            sum += static_cast<uint32_t>(raw_values[i]) << 8;
        }
    }

    return sum;
}

inline uint16_t checksum_finalize(uint32_t sum){
    while(sum >> 16){
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

inline uint16_t checksum_finalize_nz(uint32_t sum){
    auto checksum = checksum_finalize(sum);

    if(!checksum){
        return ~checksum;
    } else {
        return checksum;
    }
}

} // end of network namespace

#endif
