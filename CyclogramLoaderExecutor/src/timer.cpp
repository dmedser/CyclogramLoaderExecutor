#include "timer.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include <stdbool.h>

volatile static uint32_t msec = 0;

#define F_CPU 16000000UL  // 16 ћ√ц

void delay_timer_init() {
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = (float)F_CPU/1000;  /* —чет по миллисекунде */
	TCCR1B |= (1 << WGM12);		/* CTC mode, TOP = OCR1A */
	TIFR1 = 0;
}

void count_to(uint16_t s, uint16_t ms) {
	msec = ((uint32_t)s*1000 + ms);
	if(msec != 0) {
		TCCR1B |= (1 << 0);
		while(msec != 0);
	}
}

ISR(TIMER1_COMPA_vect) {
	if(msec != 0) {
		msec--;
	}
	if(msec == 0) {
		TCCR1B &= ~(1 << 0);
		TCNT1 = 0;
	}
}

void log_timer_init() {
	TIMSK3 |= (1 << TOIE3);
	TCCR3B |= 0x1;
}
