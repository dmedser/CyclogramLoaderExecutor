#include "avr/interrupt.h"
#include "uart_config.h"
#include "cyclogram.h"
#include "commands.h"
#include "timer.h"
#include <stddef.h>

#define CMD_STACK_CAPACITY		(2)
#define CMD_STACK_BASE			(0x15DC)

typedef enum CyclogramState : uint8_t {
	STATE_EXTERN_CMD_WAITING,
	STATE_CMD_PREPARING,
	STATE_CMD_DELAYING,
	STATE_CMD_EXECUTING,
	STATE_CYCLOGRAM_PAUSE,
	STATE_CYCLOGRAM_STOP
} CyclogramState;

volatile static CyclogramState currCyclogramState = STATE_CMD_PREPARING;
volatile static CyclogramState prevCyclogramState;

#define LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET	(2) 
#define COUNT_OF_PRESET_COMMANDS			(4)

/*
#define EXT_CMD_BASE (0x23E) 

volatile static void *extCmdBaseAddress = (void *)EXT_CMD_BASE;
volatile static uint16_t *extCmdWordPtr = (uint16_t *)extCmdBaseAddress;
volatile static Command *extCmd;

volatile static uint8_t byteCounter = 0;
volatile static uint16_t extCmdWord = 0;

ISR(USART1_RX_vect) {
	byteCounter++;
	if(byteCounter < sizeof(uint16_t)) {
		extCmdWord = UDR1;
	}
	else {
		extCmdWord <<= 8;
		extCmdWord |= UDR1;
		if(extCmdWord == HEADER) {
			extCmd = (Command *)extCmdBaseAddress;
			extCmdWordPtr = (uint16_t *)extCmdBaseAddress;
			currCmdState = EXT_CMD_RECEIVED;
		}
		else {
			*(extCmdWordPtr++) = extCmdWord;
			byteCounter = 0;
		}
	}
}
*/

uint8_t * Command::getCmdDataFromOffset(uint16_t offset) {
	return (this->data + offset);
}

uint16_t Command::get2BytesForm(uint8_t *source) {
	return *((uint16_t *)source);
}

void Command::execute(Cyclogram *cyclogram) {
	uart_transmit_16(num);
	if(id == ID_STOP) {
		currCyclogramState = STATE_CYCLOGRAM_STOP;
		return;
	}
	if(id == ID_PAUSE) {
		currCyclogramState = STATE_CYCLOGRAM_PAUSE;
		return;
	}
	uint16_t currCmdFstParam = get2BytesForm(data);
	if(id == ID_LOOP && currCmdFstParam == PARAM_LOOP_START) {
		/* „тобы при возврате на точку входа в цикл команда не заносилась в стек снова */ 
		if(this != *(cyclogram->cmdStack.peek()->loopEntryIterator)) { 
			uint16_t countOfIterations = get2BytesForm(getCmdDataFromOffset(LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET));
			cyclogram->cmdStack.push(IteratorAndCount(cyclogram->it, (countOfIterations - 1)));
		}
		++cyclogram->it;
		currCyclogramState = STATE_CMD_PREPARING;
		return;
	}
	if(id == ID_LOOP && currCmdFstParam == PARAM_LOOP_END) {
		IteratorAndCount *lastLoopEntry = cyclogram->cmdStack.peek();
		if(lastLoopEntry->countOfIterations > 0) {
			lastLoopEntry->countOfIterations--;
			cyclogram->it = lastLoopEntry->loopEntryIterator;
		}
		else {
			cyclogram->cmdStack.pop();
			++cyclogram->it;
		}
		currCyclogramState = STATE_CMD_PREPARING;
		return;
	}
	(*(cyclogram->cmdsImp[id - COUNT_OF_PRESET_COMMANDS]))(data); 
	++(cyclogram->it);
	currCyclogramState = STATE_CMD_PREPARING;
}

IteratorAndCount::IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations):
	loopEntryIterator(loopEntryIterator), 
	countOfIterations(countOfIterations) 
	{}

Cyclogram::Cyclogram(void *baseAddress, CmdImplementation *cmdsImp):
	baseAddress(baseAddress),
	it(baseAddress), 
	cmdStack((void *)CMD_STACK_BASE, CMD_STACK_CAPACITY),
	cmdsImp(cmdsImp)
	{}

volatile static uint32_t startCmdDelayMsecCount;
volatile static uint32_t msecToDelay;

void Cyclogram::run() {
	switch(currCyclogramState) {
		case STATE_CMD_PREPARING: {
			Command *currCmd = *it;
			msecToDelay = (uint32_t)currCmd->time_s*1000 + currCmd->time_ms;
			startCmdDelayMsecCount = msecCount;
			currCyclogramState = STATE_CMD_DELAYING;
			break;
		}
		case STATE_CMD_DELAYING: {
			if((msecCount - startCmdDelayMsecCount) >= msecToDelay) {
				currCyclogramState = STATE_CMD_EXECUTING;
			}
			break;
		}
		case STATE_CMD_EXECUTING: {
			/* »нкремент итератора и смена состо€ни€ внутри execute() */
			Command *currCmd = *it;
			currCmd->execute(this);
			break;
		}
		case STATE_CYCLOGRAM_STOP: {
			// TODO
			break;
		}
		case STATE_CYCLOGRAM_PAUSE: {
			// TODO
			break;
		}
		default: break;
	}
}

Cyclogram::Iterator::Iterator(void *address):
	address(address) 
	{}

Command* Cyclogram::Iterator::operator *() {
	return (Command*)(address);
}

Cyclogram::Iterator& Cyclogram::Iterator::operator ++() {
	address += offsetof(Command, data) + (*(*this))->len;
	return *this;
}


Cyclogram::Iterator& Cyclogram::Iterator::operator =(const Iterator &anotherIterator) {
	this->address = anotherIterator.address;
	return *this;
}


Cyclogram::CmdStack::CmdStack(void *base, size_t capacity) {
	this->base = (IteratorAndCount *)base;
	curr_element = this->base;
	this->capacity = capacity;
	size = 0;
}

bool Cyclogram::CmdStack::isEmpty() {
	return (size == 0);
}

bool Cyclogram::CmdStack::isFull() {
	return (size == capacity);	
}

void Cyclogram::CmdStack::push(const IteratorAndCount &newElement) {
	if (!(this->isFull())) {
		*(curr_element++) = newElement;
		size++;
	}
}

IteratorAndCount* Cyclogram::CmdStack::pop() {
	if(!(this->isEmpty())) {
		size--;
		return --curr_element;
	}
	else {
		return nullptr;
	}
}

IteratorAndCount* Cyclogram::CmdStack::peek() {
	if(!(this->isEmpty())) {
		IteratorAndCount *tmp = curr_element;
		return --tmp;
	}
	else { 
		return nullptr;
	}
}

