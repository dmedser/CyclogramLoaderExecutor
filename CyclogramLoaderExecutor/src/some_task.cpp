#include "some_task.h"
#include "timer.h"
#include <avr/io.h>

typedef enum MeanderState : uint8_t {
	MEANDER_DELAY_START,
	MEANDER_DELAYING
} MeanderState;

volatile static MeanderState currMeanderState = MEANDER_DELAY_START;
volatile static uint32_t startMeanderDelayMsecCount;

#define MEANDER_MS_PERIOD (5)


void meander() {
	switch(currMeanderState) {
		case MEANDER_DELAY_START: {
			startMeanderDelayMsecCount = msecCount;
			currMeanderState = MEANDER_DELAYING;
			break;
		}
		case MEANDER_DELAYING: {
			if((msecCount - startMeanderDelayMsecCount) >= MEANDER_MS_PERIOD) {
				if(PORTD & (1 << PD4)) {
					PORTD &= ~(1 << PD4);
				}
				else {
					PORTD |= (1 << PD4);
				}
				currMeanderState = MEANDER_DELAY_START;
			}
			break;
		}
		default: break;
	}
}
