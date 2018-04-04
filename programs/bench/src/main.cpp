//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/print.hpp>
#include <tlib/system.hpp>

constexpr const size_t PAGES = 512;

namespace {

size_t repeat = 1;

bool display_result(const char* name, uint64_t duration){
    if(!duration){
        repeat *= 2;
        tlib::printf("%s was too fast (duration=0) increasing repeat to %u\n", name, repeat);
        return false;
    }

    uint64_t throughput = 1000 * ((repeat * PAGES * 4096) / duration);

    if(throughput > (1024 * 1024)){
        tlib::printf("%s: %ums bandwith: %uMiB/s\n", name, duration, throughput / (1024 * 1024));
    } else if(throughput > 1024){
        tlib::printf("%s: %ums bandwith: %uKiB/s\n", name, duration, throughput / 1024);
    } else {
        tlib::printf("%s: %ums bandwith: %uB/s\n", name, duration, throughput);
    }

    return true;
}

} // end of anonymous namespace

int main(){
    char* buffer_one = new char[PAGES * 4096];
    char* buffer_two = new char[PAGES * 4096];

    tlib::printf("Start benchmark...\n");

    uint64_t start = 0, end = 0;

    while(repeat < 100){
        start = tlib::ms_time();

        for(size_t i = 0; i < repeat; ++i){
            std::copy_n(buffer_two, PAGES * 4096, buffer_one);
        }

        end = tlib::ms_time();

        if(display_result("copy", end - start)){
            break;
        }
    }

    repeat = 1;

    while(repeat < 100){
        start = tlib::ms_time();

        for(size_t i = 0; i < repeat; ++i){
            std::fill_n(buffer_two, PAGES * 4096, 'Z');
        }

        end = tlib::ms_time();

        if(display_result("fill", end - start)){
            break;
        }
    }

    repeat = 1;

    while(repeat < 100){
        start = tlib::ms_time();

        for(size_t i = 0; i < repeat; ++i){
            std::fill_n(buffer_two, PAGES * 4096, 0);
        }

        end = tlib::ms_time();

        if(display_result("clear", end - start)){
            break;
        }
    }

    return 0;
}
