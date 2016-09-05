//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/print.hpp>

int main(int argc, char* argv[]){
    for(size_t i = 0; i < size_t(argc); ++i){
        tlib::print_line(argv[i]);
    }

    return 0;
}
