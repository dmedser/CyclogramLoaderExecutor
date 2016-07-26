#include "error_types.h"
#include "uart_config.h"
#include "cyclogram.h"
#include <avr/io.h>
#include "timer.h"
#include "FreeRTOS.h"
#include "task.h"

#define CYCLOGRAM_BASE_ADDRESS 0x3F0
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

void vStartCyclogram(void *pvParameters) {
	Cyclogram *cyclogram = (Cyclogram *)pvParameters;
	cyclogram->run();
}

void vWaitSomeTicks(void * pvParameters) {
	TickType_t ticksToDelay = *((TickType_t *)pvParameters); 
	vTaskDelay(ticksToDelay);
	uart_transmit_16(0xBBBB);
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
	
	DDRD = 0xFF;


	
	// LOOP TEST
	*(a16++) = 1;		// num
	*(a16++) = LOOP;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;	// tms
	*(a16++) = 4;		// len 
	*(a16++) = START;
	*(a16++) = 0x0032;
	
	*(a16++) = 2;
	*(a16++) = 0xB888;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 4;
	*(a16++) = 0xC199;
	*(a16++) = 0x0001;
	
	*(a16++) = 3;		// num
	*(a16++) = 0xA999;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;	// tms
	*(a16++) = 2;		// len
	char *a8 = (char *)a16;
	*(a8++) = 'B';
	*(a8++) = 1;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 4;		// num
	*(a16++) = 0xA998;	// id
	*(a16++) = 1;		// ts
	*(a16++) = 0;	// tms
	*(a16++) = 2;		// len
	a8 = (char *)a16;
	*(a8++) = 'F';
	*(a8++) = 1;
	
	a16 = (uint16_t *)a8;
	*(a16++) = 5;
	*(a16++) = 0xFFFF;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 2;
	*(a16++) = 0xFEEE;
	
	*(a16++) = 6;
	*(a16++) = LOOP;
	*(a16++) = 1;
	*(a16++) = 0;
	*(a16++) = 2;
	*(a16++) = END;
	
	*(a16++) = 8;		// num
	*(a16++) = STOP;	// id 

	log_timer_init();	
	
	Cyclogram cyclogram((void *)CYCLOGRAM_BASE_ADDRESS);
	
	TickType_t ticksToDelay = 2000;
	 
	delay_timer_init(); 
	cyclogram.run();
	xTaskCreate(vStartCyclogram, "StartCyclogram", configMINIMAL_STACK_SIZE, &cyclogram, 1, NULL);
	//xTaskCreate(vWaitSomeTicks, "WaitSomeTicks", configMINIMAL_STACK_SIZE, &ticksToDelay, );
	vTaskStartScheduler();

    while (true) {
		
	} 
}

 