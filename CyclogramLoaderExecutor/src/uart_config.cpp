#include "uart_config.h"
#include "assertion.h"
#include "stddef.h"
#include "avr/io.h"
#include <avr/interrupt.h>



uint8_t init_uart(const struct uart_init* init_struct, uint32_t uart_freq)
{
	RT_ASSERT(init_struct != NULL, ERROR_TYPE_NULL_PTR);
	RT_ASSERT(init_struct->baud_rate > 0 && init_struct->baud_rate <= 115200, 
		  ERROR_TYPE_WRONG_PARAM);
	RT_ASSERT(assertCorrectDataBits(init_struct->data_bits), 
		  ERROR_TYPE_WRONG_PARAM);
	RT_ASSERT(assertCorrectStopBits(init_struct->stop_bits), 
		  ERROR_TYPE_WRONG_PARAM);
	RT_ASSERT(uart_freq > 0, ERROR_TYPE_WRONG_PARAM);
	
	uint8_t data_bits = 0;
	
	switch (init_struct->data_bits) {
	case UART_DATA_BITS_5 : data_bits = 0; break;
	case UART_DATA_BITS_6 : data_bits = 1; break;
	case UART_DATA_BITS_7 : data_bits = 2; break;
	case UART_DATA_BITS_8 : data_bits = 3; break;
	case UART_DATA_BITS_9 : data_bits = 7; break;
	default: break; //unreacheable
	}
	
	uint8_t stop_bits = 0;
	
	switch (init_struct->stop_bits) {
	case UART_STOP_BITS_1 : stop_bits = 0; break;
	case UART_STOP_BITS_2 : stop_bits = 1; break;
	default: break; //unreacheable
	}
	
	/* BaudRateDel = freq/(16*BaudRate) - 1
	 * Округление - (int) (x + 0.5f)
	 */
	uint16_t ubbr = (uint16_t)(((float)(uart_freq)/(16*init_struct->baud_rate) - 1) + 0.5f);
	UCSR1C = (data_bits << UCSZ10) | (stop_bits << USBS1);
	UBRR1L = (ubbr  & 0x00FF);
	UBRR1H = (ubbr & 0x0F00) >> 8;
	
	return ERROR_TYPE_OK;
}

uint8_t enable_uart(void)
{
	RT_ASSERT(UBRR1H != 0 || UBRR1L != 0, ERROR_TYPE_BAUD_RATE_NOT_SET);
	UCSR1B = (1 << RXEN1) | (1 << TXEN1);
	UCSR1B |= (1 << RXCIE1);
	return ERROR_TYPE_OK;
}

/* 
 * Бит UDRE1 устанавливается в 1 по готовности передачи 
 * Ожидание готовности передачи, пока UDRE1 == 0 
 */ 
void uart_transmit_8(uint8_t data) 
{
	while(!(UCSR1A & (1 << UDRE1)));
	UDR1 = data;
}

void uart_transmit_16(uint16_t data)
{
	uint8_t *byte_ptr;
	byte_ptr = (uint8_t *)&data;
	for (uint8_t i = 0; i < sizeof(uint16_t); i++) {
		uart_transmit_8(*byte_ptr++);
	}
}


void uart_transmit_32(uint32_t data)
{
	uint8_t *byte_ptr;
	byte_ptr = (uint8_t *)&data;
	for (uint8_t i = 0; i < sizeof(uint32_t); i++) {
		uart_transmit_8(*byte_ptr++);
	}
}

/*
void uart_transmit_float(float data)
{
	uint8_t *byte_ptr;
	byte_ptr = (uint8_t *)&data;
	for (uint8_t i = 0; i < sizeof(float); i++) {
		uart_transmit(*byte_ptr++);
	}
}
*/

#define HEADER	(0x7C6E)

/* Назначение переменных описано в заголовочном файле ядра - cyclogram.h */
volatile uint16_t *extCmdWordPtr = (uint16_t *)EXT_CMD_BASE_ADDRESS;
volatile bool extCmdIsReceived = false;
volatile Command *extCmd;

volatile uint8_t byteCounter = 0;
volatile uint16_t extCmdWord;

ISR(USART1_RX_vect) {
	byteCounter++;
	if(byteCounter < sizeof(uint16_t)) {
		extCmdWord = UDR1;
	}
	else {
		extCmdWord <<= 8;
		extCmdWord |= UDR1;
		if(extCmdWord == HEADER) {
			extCmdIsReceived = true;
		}
		else {
			*(extCmdWordPtr++) = extCmdWord;
		}
		byteCounter = 0;
	}
}