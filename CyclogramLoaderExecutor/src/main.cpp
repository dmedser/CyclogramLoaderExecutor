#include "error_types.h"
#include "uart_config.h"
#include "cyclogram.h"
#include <avr/io.h>
#include "timer.h"

#define BASE_SRAM 0x0210
#define SRAM_END  0x41FF

/* Частота тактирования CPU = 16 МГц */
#define F_CPU (16000000)

void check_error(uint8_t error)
{
	volatile uint8_t error_type = error;
	if (error_type == ERROR_TYPE_OK) {
		return;
	}
	while(true);
}

int main(void)
{	
	uint8_t ret_val;
	/* Глобальное разрешение прерываний */
	SREG |= (1 << SREG_I);
	/* Инициализация UART */
	{
		struct uart_init uart_init;
		uart_init.baud_rate = 115200;
		uart_init.stop_bits = UART_STOP_BITS_1;
		uart_init.data_bits = UART_DATA_BITS_8;
		ret_val = init_uart(&uart_init, F_CPU);
		check_error(ret_val);
		ret_val = enable_uart();
		check_error(ret_val);
	}

	
	volatile uint16_t *a16 = (uint16_t *)BASE_SRAM;
	
	DDRD = 0xFF;
	/*
	// LDI ADD TEST
	*(a16++) = 1;		//num
	*(a16++) = LDI;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 1;		//tms
	*(a16++) = 2;		//len
	char *a8 = (char *)a16;
	*(a8++) = 'B';
	*(a8++) = 10;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 2;		//num
	*(a16++) = LDI;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 1;		//tms
	*(a16++) = 2;		//len
	a8 = (char *)a16;
	*(a8++) = 'D';
	*(a8++) = 11;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 3;		//num
	*(a16++) = ADD;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 1;		//tms
	*(a16++) = 2;		//len
	a8 = (char *)a16;
	*(a8++) = 'D';
	*(a8++) = 'B';
	
	a16 = (uint16_t *)a8;
	*(a16++) = 4;    // num
	*(a16++) = STOP; // id
	*(a16++) = 0;    // ts
	*(a16++) = 0;    // tms
	*(a16++) = 0;    //len
	*/
	
	
	// LOOP TEST
	*(a16++) = 1;		//num
	*(a16++) = LOOP;	//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 4;		//len
	*(a16++) = START;
	*(a16++) = 0x000A;
	
	*(a16++) = 2;
	*(a16++) = LOOP;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 4;
	*(a16++) = START;
	*(a16++) = 0x0001;
	
	*(a16++) = 3;		//num
	*(a16++) = LDI;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 2;		//len
	char *a8 = (char *)a16;
	*(a8++) = 'B';
	*(a8++) = 10;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 4;		//num
	*(a16++) = LDI;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 2;		//len
	a8 = (char *)a16;
	*(a8++) = 'F';
	*(a8++) = 11;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 5;		//num
	*(a16++) = ADD;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 2;		//len
	a8 = (char *)a16;
	*(a8++) = 'F';
	*(a8++) = 'B';
	
	
	
	a16 = (uint16_t *)a8;
	*(a16++) = 6;
	*(a16++) = LOOP;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 2;
	*(a16++) = END;
	
	
	*(a16++) = 7;
	*(a16++) = LOOP;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 2;
	*(a16++) = END;
	
	*(a16++) = 8;		// num
	*(a16++) = STOP;	// id 
	*(a16++) = 1;		// ts
	*(a16++) = 0;	// tms
	*(a16++) = 0;		//len
	
	
	
	/*
	// STACK TEST
	*(a16++) = 0;		// num
	*(a16++) = LOOP;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;		// tms
	*(a16++) = 4;		// len
	*(a16++) = START;
	*(a16++) = 0x0002;
	
	*(a16++) = 1;		//num		
	*(a16++) = 1;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 4;		//len
	*(a16++) = 0x0fd0;	
	*(a16++) = 0x0002;
	
	*(a16++) = 2;		//num
	*(a16++) = 3;		//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 4;		//len
	*(a16++) = 0x0fd0;
	*(a16++) = 0x0002;
	
	*(a16++) = 3;		//num
	*(a16++) = LOOP;	//id
	*(a16++) = 1;		//ts
	*(a16++) = 0;		//tms
	*(a16++) = 2;		//len
	*(a16++) = END; 
	
	*(a16++) = 4;	 // num
	*(a16++) = STOP; // id
	*(a16++) = 1;	 // ts
	*(a16++) = 0;	 // tms
	*(a16++) = 0;	 // len
	*/
	
	Cyclogram cyclogram((void *)BASE_SRAM);
	log_timer_init();
	cyclogram.run();

    while (1) 
    {
    }
}

