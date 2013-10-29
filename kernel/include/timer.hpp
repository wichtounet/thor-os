#ifndef TIMER_H
#define TIMER_H

#include <cstddef>

void install_timer();
void sleep_ms(std::size_t delay);

std::size_t timer_ticks();
std::size_t timer_seconds();

#endif
