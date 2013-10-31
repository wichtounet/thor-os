#include "memory.hpp"
#include "timer.hpp"
#include "shell.hpp"

extern "C" {
void  __attribute__ ((section ("main_section"))) kernel_main(){
    load_memory_map();
    init_shell();
    install_timer();

    return;
}
}
