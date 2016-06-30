#include <avr/io.h>
#include "cyclogram.h"
#define BASE_SRAM 0x0200
#define SRAM_END  0x41FF

int main(void)
{
	/*
	volatile Command *command1 = *it;
	++it;
	volatile Command *command2 = *it;
	
	volatile uint8_t* ptr2 = command2->data;
	volatile uint8_t fst2 = *(ptr2++);
	volatile uint8_t sd2 = *(ptr2++);
	volatile uint8_t thrd2 = *(ptr2++);
	volatile uint8_t frth2 = *ptr2;
	*/
	volatile uint8_t *a8 = (uint8_t *)BASE_SRAM;
	volatile uint16_t *a16 = (uint16_t *)a8;
	*(a16++) = 1;
	*(a16++) = 2;
	*(a16++) = 3;
	*(a16++) = 4;
	*(a16++) = 4;
	*(a16++) = 0xEF98;
	*(a16++) = 0xABCD;
	
	*(a16++) = 99;
	*(a16++) = 98;
	*(a16++) = 97;
	*(a16++) = 96;
	*(a16++) = 2;
	*(a16++) = 0x9999;
	*a16 = 0x8888;
	
	
	
	Cyclogram cyclogram((void *)BASE_SRAM);
	Cyclogram::Iterator it((void *)BASE_SRAM);
	Cyclogram::CmdStack cmdStack((void *)0x5E8, (void *)0x5EA);
	cmdStack.push(it.getCurrAddress());
	volatile void *fst_amd_addr = cmdStack.peek();
	++it;
	cmdStack.push(it.getCurrAddress());
	volatile void *scd_amd_addr = cmdStack.peek();
	
    while (1) 
    {
    }
}

