#include "memory.hpp"
#include "timer.hpp"
#include "shell.hpp"
#include "keyboard.hpp"

extern "C" {

void  __attribute__ ((section ("main_section"))) kernel_main(){
    load_memory_map();
    init_memory_manager();
    install_timer();
    keyboard::install_driver();
    init_shell();

    return;
}

}
