#include "commands.h"
#include "cyclogram.h"
#include "uart_config.h"
#include <avr/io.h>


void ldi(uint8_t *paramPtr) {
	char portName = (char)*(paramPtr);
	uint8_t val = *(paramPtr + CMD_OFFSET_LDI_VALUE);
	switch(portName) {
		case 'B': {DDRB = 0xFF; PORTB = val; break;}
		case 'D': {DDRD = 0xFF; PORTD = val; break;}
		default: break;
	}
	uart_transmit_16(CMD_ID_LDI);
}

/* ADD Rd, Rr  =  Rd <- Rd + Rr */
void add(uint8_t *paramPtr) {
	char Rd = (char)*(paramPtr);
	char Rr = (char)*(paramPtr + CMD_OFFSET_ADD_SECOND_REG);
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
	uart_transmit_16(CMD_ID_ADD);
}

void sbi(uint8_t *paramPtr) {
    char portName = (char)*paramPtr;
    uint8_t bitNo = *(paramPtr + CMD_OFFSET_BITWISE_BIT_NUMBER);
    switch(portName) {
	    case 'D': {DDRD = 0xFF; PORTD |= (1 << bitNo); break;}
	    case 'F': {DDRF = 0xFF; PORTF |= (1 << bitNo); break;}
	    default: break;
	}
	uart_transmit_16(CMD_ID_SBI);
}

void cbi(uint8_t *paramPtr) {
	char portName = (char)*paramPtr;
	uint8_t bitNo = *(paramPtr + CMD_OFFSET_BITWISE_BIT_NUMBER);
	switch(portName) {
		case 'D': {DDRD = 0xFF; PORTD &= ~(1 << bitNo); break;}
		case 'F': {DDRF = 0xFF; PORTF &= ~(1 << bitNo); break;}
		default: break;
	}	
	uart_transmit_16(CMD_ID_CBI);
}
 
