#ifndef TIMER_H
#define TIMER_H

#include "types.hpp"

void install_timer();
void sleep_ms(uint64_t delay);

uint64_t timer_ticks();
uint64_t timer_seconds();

#endif
