//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#include <tlib/file.hpp>
#include <tlib/system.hpp>
#include <tlib/errors.hpp>
#include <tlib/print.hpp>

std::string read_file(const std::string& path, bool check = true){
    tlib::file f(path);

    if(!f){
        if(check){
            tlib::printf("ifconfig: error: %s\n", std::error_message(f.error()));
        }

        return "";
    }

    auto contents = f.read_file();

    if(!f){
        tlib::printf("ifconfig: error: %s\n", std::error_message(f.error()));
    }

    return contents;
}

int main(int /*argc*/, char* /*argv*/[]){
    tlib::file dir("/sys/net/");

    if(!dir){
        tlib::printf("ifconfig: error: %s\n", std::error_message(dir.error()));
        return 1;
    }

    bool first = true;
    for(auto entry_name : dir.entries()){
        if(!first){
            tlib::printf("\n");
        }

        first = false;

        std::string base_path = "/sys/net/";

        auto enabled = read_file(base_path + entry_name + "/enabled");

        if(enabled == "true"){
            auto driver = read_file(base_path + entry_name + "/driver");
            auto ip = read_file(base_path + entry_name + "/ip");
            auto gw = read_file(base_path + entry_name + "/gateway", false);
            auto mac = read_file(base_path + entry_name + "/mac");

            tlib::printf("%10s inet %s\n", entry_name, ip.c_str());

            if(!gw.empty()){
                tlib::printf("%10s   gw %s\n", "", gw.c_str());
            }

            if(!mac.empty() && mac != "0"){
                tlib::printf("%10s ether %s\n", "", mac.c_str());
            }

            tlib::printf("%10s driver %s\n", "", driver.c_str());
        }
    }

    return 0;
}
