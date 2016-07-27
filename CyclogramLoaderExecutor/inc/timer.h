#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>

void msec_timer_init();

extern volatile uint32_t msecCount;

#endif /* TIMER_H_ */