#include "error_types.h"
#include "uart_config.h"
#include "cyclogram.h"
#include "some_task.h"
#include "commands.h"
#include "timer.h"
#include <avr/io.h>


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
	
	volatile uint16_t *a16 = (uint16_t *)CYCLOGRAM_BASE_ADDRESS;
	
	// LOOP TEST
	*(a16++) = 1;		// num
	*(a16++) = CMD_ID_LOOP;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;		// tms
	*(a16++) = 4;		// len 
	*(a16++) = CMD_PARAM_LOOP_START;
	*(a16++) = 0x0004;
	
	*(a16++) = 2;
	*(a16++) = CMD_ID_LDI;
	*(a16++) = 1;		// ts
	*(a16++) = 0;		// tms
	*(a16++) = 2;		// len
	char *a8 = (char *)a16;
	*(a8++) = 'B';
	*(a8++) = 1;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 3;		// num
	*(a16++) = CMD_ID_ADD;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;		// tms
	*(a16++) = 2;		// len
	a8 = (char *)a16;
	*(a8++) = 'D';
	*(a8++) = 'B';
	
	a16 = (uint16_t *)a8;
	*(a16++) = 4;		// num
	*(a16++) = CMD_ID_SBI;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;		// tms
	*(a16++) = 2;		// len
	a8 = (char *)a16;
	*(a8++) = 'F';
	*(a8++) = 1;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 5;		// num
	*(a16++) = CMD_ID_CBI;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;		// tms
	*(a16++) = 2;		// len
	a8 = (char *)a16;
	*(a8++) = 'F';
	*(a8++) = 1;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 6;
	*(a16++) = CMD_ID_LOOP;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 2;
	*(a16++) = CMD_PARAM_LOOP_END;
	
	*(a16++) = 7;		// num
	*(a16++) = CMD_ID_STOP;	// id 
	
	
	
	CmdImplementation cmds[COMMANDS_NUMBER] = {&ldi, &add, &sbi, &cbi};

	Cyclogram cyclogram((void *)CYCLOGRAM_BASE_ADDRESS, cmds);
	
	msec_timer_init(); 

    while (true) {
		cyclogram.run();
		meander();
	} 
}

 