//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>
#include <datetime.hpp>

constexpr const size_t PAGES = 512;

namespace {

size_t repeat = 1;

bool display_result(const char* name, uint64_t duration){
    if(!duration){
        repeat *= 2;
        printf("%s was too fast (duration=0) increasing repeat to %u\n", name, repeat);
        return false;
    }

    uint64_t throughput = 1000 * ((repeat * PAGES * 4096) / duration);

    if(throughput > (1024 * 1024)){
        printf("%s: %ums bandwith: %uMiB/s\n", name, duration, throughput / (1024 * 1024));
    } else if(throughput > 1024){
        printf("%s: %ums bandwith: %uKiB/s\n", name, duration, throughput / 1024);
    } else {
        printf("%s: %ums bandwith: %uB/s\n", name, duration, throughput);
    }

    return true;
}

} // end of anonymous namespace

int main(){
    char* buffer_one = new char[PAGES * 4096];
    char* buffer_two = new char[PAGES * 4096];

    printf("Start benchmark...\n");

    uint64_t start = 0, end = 0;

    while(repeat < 100){
        start = ms_time();

        for(size_t i = 0; i < repeat; ++i){
            std::copy_n(buffer_one, buffer_two, PAGES * 4096);
        }

        end = ms_time();

        if(display_result("copy", end - start)){
            break;
        }
    }

    repeat = 1;

    while(repeat < 100){
        start = ms_time();

        for(size_t i = 0; i < repeat; ++i){
            std::fill_n(buffer_two, PAGES * 4096, 'Z');
        }

        end = ms_time();

        if(display_result("fill", end - start)){
            break;
        }
    }

    repeat = 1;

    while(repeat < 100){
        start = ms_time();

        for(size_t i = 0; i < repeat; ++i){
            std::fill_n(buffer_two, PAGES * 4096, 0);
        }

        end = ms_time();

        if(display_result("clear", end - start)){
            break;
        }
    }

    return 0;
}
