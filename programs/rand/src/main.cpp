//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <random.hpp>

#include <tlib/system.hpp>
#include <tlib/print.hpp>

int main(){
    std::default_random_engine engine(tlib::ms_time());
    std::uniform_int_distribution<> dist(0, 100);

    tlib::printf("%d\n", dist(engine));

    return 0;
}
