//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <print.hpp>
#include <system.hpp>
#include <datetime.hpp>

constexpr const size_t PAGES = 256;

int main(){
    char* buffer_one = new char[PAGES * 4096];
    char* buffer_two = new char[PAGES * 4096];

    auto start = ms_time();

    std::copy_n(buffer_one, buffer_two, PAGES * 4096);

    auto end = ms_time();

    auto copy_duration = end - start;
    auto copy_throughput = 1000 * (PAGES * 4096) / copy_duration;

    if(copy_throughput > 1024 * 1024){
        printf("copy: %ums bandwith: %uMiB/s\n", copy_duration, copy_throughput / 1024 * 1024);
    } else if(copy_throughput > 1024){
        printf("copy: %ums bandwith: %uKiB/s\n", copy_duration, copy_throughput / 1024);
    } else {
        printf("copy: %ums bandwith: %uB/s\n", copy_duration, copy_throughput);
    }

    start = ms_time();

    std::fill_n(buffer_two, PAGES * 4096, 'Z');

    end = ms_time();

    auto fill_duration = end - start;
    auto fill_throughput = 1000 * (PAGES * 4096) / fill_duration;

    if(fill_throughput > 1024 * 1024){
        printf("fill: %ums bandwith: %uMiB/s\n", fill_duration, fill_throughput / 1024 * 1024);
    } else if(fill_throughput > 1024){
        printf("fill: %ums bandwith: %uKiB/s\n", fill_duration, fill_throughput / 1024);
    } else {
        printf("fill: %ums bandwith: %uB/s\n", fill_duration, fill_throughput);
    }

    start = ms_time();

    std::fill_n(buffer_two, PAGES * 4096, 0);

    end = ms_time();

    auto clear_duration = end - start;
    auto clear_throughput = 1000 * (PAGES * 4096) / clear_duration;

    if(clear_throughput > 1024 * 1024){
        printf("clear: %ums bandwith: %uMiB/s\n", clear_duration, clear_throughput / 1024 * 1024);
    } else if(clear_throughput > 1024){
        printf("clear: %ums bandwith: %uKiB/s\n", clear_duration, clear_throughput / 1024);
    } else {
        printf("clear: %ums bandwith: %uB/s\n", clear_duration, clear_throughput);
    }

    exit(0);
}
