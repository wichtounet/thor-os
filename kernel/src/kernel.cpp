#include "types.hpp"
#include "keyboard.hpp"
#include "kernel_utils.hpp"
#include "console.hpp"
#include "timer.hpp"
#include "shell.hpp"

void keyboard_handler();
void clear_command();

extern "C" {
void  __attribute__ ((section ("main_section"))) kernel_main(){
    init_shell();
    install_timer();

    return;
}
}
