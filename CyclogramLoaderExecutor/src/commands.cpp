#include "commands.h"
#include <avr/io.h>
#define CMD_LDI_VALUE_OFFSET			(1)
#define CMD_ADD_SECOND_REG_OFFSET		(1)
#define CMD_BITWISE_BIT_NUMBER_OFFSET	(1)


void ldi(uint8_t *paramPtr) {
	char portName = (char)*(paramPtr);
	uint8_t val = *(paramPtr + CMD_LDI_VALUE_OFFSET);
	switch(portName) {
		case 'B': {DDRB = 0xFF; PORTB = val; break;}
		case 'D': {DDRD = 0xFF; PORTD = val; break;}
		default: break;
	}
}

/* ADD Rd, Rr  =  Rd <- Rd + Rr */
void add(uint8_t *paramPtr) {
	char Rd = (char)*(paramPtr);
	char Rr = (char)*(paramPtr + CMD_ADD_SECOND_REG_OFFSET);
	uint8_t RrVal;
	switch(Rr) {
		case 'B': {RrVal = PORTB; break;}
		case 'D': {RrVal = PORTD; break;}
		default: break;
	}
	switch(Rd) {
		case 'B': {DDRB = 0xFF; PORTB += RrVal; break;}
		case 'D': {DDRD = 0xFF; PORTD += RrVal; break;}
		default: break;
	}
}

void sbi(uint8_t *paramPtr) {
    char portName = (char)*paramPtr;
    uint8_t bitNo = *(paramPtr + CMD_BITWISE_BIT_NUMBER_OFFSET);
    switch(portName) {
	    case 'D': {DDRD = 0xFF; PORTD |= (1 << bitNo); break;}
	    case 'F': {DDRF = 0xFF; PORTF |= (1 << bitNo); break;}
	    default: break;
	}
}

void cbi(uint8_t *paramPtr) {
	char portName = (char)*paramPtr;
	uint8_t bitNo = *(paramPtr + CMD_BITWISE_BIT_NUMBER_OFFSET);
	switch(portName) {
		case 'D': {DDRD = 0xFF; PORTD &= ~(1 << bitNo); break;}
		case 'F': {DDRF = 0xFF; PORTF &= ~(1 << bitNo); break;}
		default: break;
	}	
}
 
