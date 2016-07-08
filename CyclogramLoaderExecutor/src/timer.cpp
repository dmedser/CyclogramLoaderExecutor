#include "timer.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include <stdbool.h>

volatile static uint32_t msec = 0;

#define F_CPU 16000000UL  // 16 ћ√ц

void countTo(uint16_t s, uint16_t ms) {
	msec = s*1000 + ms; 
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = (float)F_CPU/1000;  /* —чет по миллисекунде */
	TCCR1B = 0;
	TCCR1B |= (1 << WGM12);		/* CTC mode, TOP = OCR1A */
	TCCR1B |= 0x1;
	while(msec != 0);
}

ISR(TIMER1_COMPA_vect) {
	if(msec != 0) {
		msec--;
	}
}