//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <type_traits.hpp>
#include <utility.hpp>
#include <types.hpp>
#include <enable_if.hpp>

namespace std {

template<typename InputIterator, typename OutputIterator>
void copy(OutputIterator out, InputIterator it, InputIterator end){
    if(it != end){
        *out = *it;

        while(++it != end){
            *++out = *it;
        }
    }
}

template<typename InputIterator, typename OutputIterator, std::enable_if_t<!__has_trivial_assign(OutputIterator), int> = 42>
void copy_n(OutputIterator out, InputIterator in, size_t n){
    if(n > 0){
        *out = *in;

        while(--n){
            *++out = *++in;
        }
    }
}

inline void memcpy(char* out, const char* in, size_t bytes){
    if(!bytes){
        return;
    }

    // Copy as much as possible 64 bits at at time
    if(bytes >= 8){
        auto* out64 = reinterpret_cast<uint64_t*>(out);
        auto* in64 = reinterpret_cast<const uint64_t*>(in);

        size_t l = 1 + bytes / 8;

        while(--l){
            *out64++ = *in64++;
        }

        bytes -= l * 8;
    }

    // Finish up byte by byte
    while(bytes >= 1){
        *out++ = *in++;
        --bytes;
    }
}

template<typename InputIterator, typename OutputIterator, std::enable_if_t<__has_trivial_assign(OutputIterator), int> = 42>
void copy_n(OutputIterator out, InputIterator in, size_t n){
    memcpy(reinterpret_cast<char*>(out), reinterpret_cast<const char*>(in), n * sizeof(decltype(*out)));
}

template<typename InputIterator, typename OutputIterator>
void move_n(OutputIterator out, InputIterator in, size_t n){
    if(n > 0){
        *out = std::move(*in);

        while(--n){
            *++out = std::move(*++in);
        }
    }
}

template<typename ForwardIterator, typename T>
void fill(ForwardIterator it, ForwardIterator end, const T& value){
    if(it != end){
        *it = value;

        while(++it != end){
            *it = value;
        }
    }
}

template<typename ForwardIterator, typename T>
void fill_n(ForwardIterator it, size_t n, const T& value){
    if(n > 0){
        *it = value;

        while(--n){
            *++it = value;
        }
    }
}

template<typename Iterator1, typename Iterator2>
size_t compare_n(Iterator1 it1, Iterator2 it2, size_t n){
    if(n > 0){
        while(n--){
            if(*it1 != *it2){
                return *it1- *it2;
            } else {
                ++it1;
                ++it2;
            }
        }
    }

    return 0;
}

template<typename Iterator1, typename Iterator2>
bool equal_n(Iterator1 it1, Iterator2 it2, size_t n){
    return compare_n(it1, it2, n) == 0;
}

template<typename T>
constexpr const T& min(const T& a, const T& b){
    return a <= b ? a : b;
}

template<typename T>
constexpr const T& max(const T& a, const T& b){
    return a >= b ? a : b;
}

} //end of namespace std

#endif
