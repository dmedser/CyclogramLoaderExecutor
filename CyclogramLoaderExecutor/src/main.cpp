#include <avr/io.h>
#include "cyclogram.h"
#include "error_types.h"
#include "uart_config.h"

#define BASE_SRAM 0x0200
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


#define STOP  (0x7C6E)
#define LOOP  (0x9FEE)
#define START (0x28C8)
#define END   (0xABCD)


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
	
	
	volatile uint8_t *a8 = (uint8_t *)BASE_SRAM;
	volatile uint16_t *a16 = (uint16_t *)a8;
	*(a16++) = 1;		//num
	*(a16++) = LOOP;	//id
	*(a16++) = 1;		//ts
	*(a16++) = 1;		//tms
	*(a16++) = 3;		//len
	*(a16++) = START;
	a8 = (uint8_t *)a16;
	*(a8++) = 2;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 2;
	*(a16++) = 2;
	*(a16++) = 2;
	*(a16++) = 2;
	*(a16++) = 2;
	*(a16++) = 0x2222;
	
	
	*(a16++) = 3;
	*(a16++) = 3;
	*(a16++) = 3;
	*(a16++) = 3;
	*(a16++) = 3;   
	*(a16++) = 0x3333;
	a8 = (uint8_t *)a16;
	*(a8++) = 0x33;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 4;		//num
	*(a16++) = LOOP;	//id
	*(a16++) = 4;		//ts
	*(a16++) = 4;		//tms
	*(a16++) = 2;		//len
	*(a16++) = END;
	
	*(a16++) = 5;
	*(a16++) = STOP;
	*(a16++) = 5;
	*(a16++) = 5;
	*(a16++) = 0;
	
	
	
	Cyclogram cyclogram((void *)BASE_SRAM);
	
	//Cyclogram::Iterator it((void *)BASE_SRAM);
	//Cyclogram::CmdStack cmdStack((void *)0x5E8, 4);
	cyclogram.run();
	
	
	
	
    while (1) 
    {
    }
}

