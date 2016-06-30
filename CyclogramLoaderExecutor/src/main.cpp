#include <avr/io.h>
#include "cyclogram.h"
#define BASE_SRAM 0x0200
#define SRAM_END  0x41FF

int main(void)
{
	
	volatile uint8_t *a8 = (uint8_t *)BASE_SRAM;
	volatile uint16_t *a16 = (uint16_t *)a8;
	*(a16++) = 1;
	*(a16++) = 1;
	*(a16++) = 1;
	*(a16++) = 1;
	*(a16++) = 1;
	a8 = (uint8_t *)a16;
	*(a8++) = 0x11;
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
	
	*(a16++) = 4;
	*(a16++) = 4;
	*(a16++) = 4;
	*(a16++) = 4;
	*(a16++) = 4;
	*(a16++) = 0x4444;
	*(a16++) = 0x4444;
	
	
	Cyclogram cyclogram((void *)BASE_SRAM);
	Cyclogram::Iterator it((void *)BASE_SRAM);
	Cyclogram::CmdStack cmdStack((void *)0x5E8, 4);
	
	cmdStack.push(it.getCurrCmdAddress());
	++it;
	cmdStack.push(it.getCurrCmdAddress());
	++it;
	cmdStack.push(it.getCurrCmdAddress());
	++it;
	cmdStack.push(it.getCurrCmdAddress());
	
	
	Command *cmd4 = cmdStack.pop();
	Command *cmd3 = cmdStack.pop();
	Command *cmd2 = cmdStack.pop();
	Command *cmd1 = cmdStack.pop();
	
	//Command *testPop1 = cmdStack.pop();
	//Command *testPop2 = cmdStack.pop();
	 
	
	/*
	volatile Command *scdCmdAddr = cmdStack.pop();
	volatile Command *fstCmdAddr = cmdStack.pop();
	*/
	/*
	uint32_t data1 = *((uint32_t *)fstCmdAddr->data);
	uint16_t data2 = *((uint16_t *)scdCmdAddr->data);
	*/
    while (1) 
    {
    }
}

