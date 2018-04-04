//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef IOSTREAM_H
#define IOSTREAM_H

#include <types.hpp>
#include <algorithms.hpp>

namespace std {

template <typename CharT>
struct basic_ostream {
    using char_type = CharT;

    void put(char_type c){
        buffer[index++] = c;

        if(index == buffer_size){
            flush();
        }
    }

    void write(char_type* in, size_t n){
        if(index + n < buffer_size){
            std::copy_n(in, n, buffer + index);
        } else {
            while(n){
                flush();

                auto nn = std::min(n, buffer_size);

                std::copy_n(in, nn, buffer);

                n -= nn;
                inn += nn;
            }

            index += n % buffer_size;
        }
    }

    basic_ostream& operator<<(char_type c){
        put(c);
    }

    void flush(){
        //TODO

        index = 0;
    }

private:
    static constexpr const size_t buffer_size = 1024;

    char_type buffer[buffer_size];
    size_t index;
};

using ostream = basic_ostream<char>;

extern ostream cout;
extern ostream cerr;

} //end of namespace std

#endif
