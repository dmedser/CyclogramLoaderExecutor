#ifndef TIMER_H_
#define TIMER_H_
#include <stdint.h>

void delay_timer_init();
//void count_to(uint16_t s, uint16_t ms);
void log_timer_init();
void vCmdDelayTask(void *pvParameters);

#endif /* TIMER_H_ */