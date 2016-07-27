#include "timer.h"
#include "avr/io.h"
#include "avr/interrupt.h"

#define F_CPU 16000000UL  // 16 ћ√ц

void msec_timer_init() {
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = (float)F_CPU/1000;  /* —чет по миллисекунде */
	TCCR1B |= (1 << WGM12);		/* CTC mode, TOP = OCR1A */
	TCCR1B |= (1 << CS10);
	TIFR1 = 0;//
}

volatile uint32_t msecCount = 0;

ISR(TIMER1_COMPA_vect) {
	msecCount++;
}