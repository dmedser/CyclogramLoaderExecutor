#include "avr/interrupt.h"
#include "uart_config.h"
#include "cyclogram.h"
#include "timer.h"

#include <util/delay.h>
#include <stddef.h>

#define EXT_CMD_BASE (0x23E) 

/*
volatile static void *extCmdBaseAddress = (void *)EXT_CMD_BASE; 
volatile static uint16_t *extCmdWordPtr = (uint16_t *)extCmdBaseAddress;
volatile static Command *extCmd;
volatile static bool extCmdIsReceived = false;
*/

#define CMD_STACK_CAPACITY						(2)
#define CMD_STACK_BASE							(0x15E8)

#define LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET	(2) 
#define LDI_CMD_VALUE_OFFSET				(1)
#define ADD_CMD_SECOND_REG_OFFSET			(1)
#define BITWISE_CMD_BIT_NUMBER_OFFSET		(1)



/*
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
			extCmdIsReceived = true;
		}
		else {
			*(extCmdWordPtr++) = extCmdWord;
			byteCounter = 0;
		}
	}
}
*/



typedef enum CmdState : uint8_t {
	CMD_EXECUTING,
	CMD_READY,
	CMD_DELAY_START,
	CMD_DELAYING,
	CYCLO_COMPLETED
} CmdState;


volatile static CmdState currCmdState = CMD_READY;

volatile static uint32_t startCmdDelayMsecCount;  
volatile static uint32_t msecToDelay;

uint8_t * Command::getCmdDataFromOffset(uint16_t offset) {
	return (this->data + offset);
}

uint16_t Command::get2BytesForm(uint8_t *source) {
	return *((uint16_t *)source);
}

void Command::execute() {
	uart_transmit_16(num);
	switch(id) {
		case LDI: {
			char portName = (char)*(data);
			uint8_t val = *(getCmdDataFromOffset(LDI_CMD_VALUE_OFFSET));
			switch(portName) { 
				case 'B': {DDRB = 0xFF; PORTB = val; break;}
				case 'F': {DDRF = 0xFF; PORTF = val; break;}
				default: break;
			}
			break;
		}
		case ADD: { // ADD Rd, Rr    =   Rd <- Rd + Rr
			char Rd = (char)*(data);
			char Rr = (char)*(getCmdDataFromOffset(ADD_CMD_SECOND_REG_OFFSET));
			uint8_t RrVal;
			switch(Rr) {
				case 'B': {RrVal = PORTB; break;}
				case 'F': {RrVal = PORTF; break;}
				default: break;
			}
			switch(Rd) {
				case 'B': {DDRB = 0xFF; PORTB += RrVal; break;}
				case 'F': {DDRF = 0xFF; PORTF += RrVal; break;}
				default: break;
			}
			break;
		}
		case SBI: {
			char portName = (char)*data;
			uint8_t bitNo = *getCmdDataFromOffset(BITWISE_CMD_BIT_NUMBER_OFFSET);
			switch(portName) {
				case 'D': {DDRD = 0xFF; PORTD |= (1 << bitNo); break;}
				case 'F': {DDRF = 0xFF; PORTF |= (1 << bitNo); break;}
				default: break;
			}
			break;
		} 
		case CBI: {
			char portName = (char)*data;
			uint8_t bitNo = *getCmdDataFromOffset(BITWISE_CMD_BIT_NUMBER_OFFSET);
			switch(portName) {
				case 'D': {DDRD = 0xFF; PORTD &= ~(1 << bitNo); break;}
				case 'F': {DDRF = 0xFF; PORTF &= ~(1 << bitNo); break;}
				default: break;
			}
			break;
		}
		default: break;
	}

}

IteratorAndCount::IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations):loopEntryIterator(loopEntryIterator), countOfIterations(countOfIterations) {}

Cyclogram::Cyclogram(void* base_address):base_address(base_address), it(base_address), cmdStack((void *)CMD_STACK_BASE, CMD_STACK_CAPACITY) {}

void Cyclogram::run() {
	//while(!extCmdIsReceived);		//
	//if(extCmd->id == START) {		//
	//	extCmdIsReceived = false;	//
	
	switch(currCmdState) {
		case CMD_READY: {
			Command *currCmd = *it;
			uint16_t currCmdId = currCmd->id;
			if(currCmdId == STOP) {
				currCmdState = CYCLO_COMPLETED;
				break;
			}
			msecToDelay = (uint32_t)currCmd->time_s*1000 + currCmd->time_ms;
			currCmdState = CMD_DELAY_START;
			break;
		}
		case CMD_DELAY_START: {
			startCmdDelayMsecCount = msecCount;
			currCmdState = CMD_DELAYING;
			break;	
		}
		case CMD_DELAYING: {
			if((msecCount - startCmdDelayMsecCount) == msecToDelay) {
				currCmdState = CMD_EXECUTING;
			}
			break;
		}
		case CMD_EXECUTING: {
			Command *currCmd = *it;
			uint16_t currCmdId = currCmd->id;
			uint16_t currCmdFstParam = currCmd->get2BytesForm(currCmd->data);
			/*
			if(extCmdIsReceived) {
				switch(extCmd->id) {
					case STOP: {break}
				}
				break;
			}
			if(extCmdWord == PAUSE) {
				while(extCmdWord != START);
			}
			*/
			if(currCmdId == LOOP && currCmdFstParam == START) {
				if(currCmd != *(cmdStack.peek()->loopEntryIterator)) {		// 
					uint16_t countOfIterations = currCmd->get2BytesForm(currCmd->getCmdDataFromOffset(LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET));
					cmdStack.push(IteratorAndCount(it, (countOfIterations - 1)));
				}															//
			}
			if(currCmdId == LOOP && currCmdFstParam == END) {
				IteratorAndCount *lastLoopEntry = cmdStack.peek();
				if(lastLoopEntry->countOfIterations > 0) {
					lastLoopEntry->countOfIterations--;
					it = lastLoopEntry->loopEntryIterator;
					currCmd->execute();       //
					currCmdState = CMD_READY; //
					break;			          //
 				}
				else {
					cmdStack.pop();		
				}
			}
			currCmd->execute();
			++it;	
			currCmdState = CMD_READY;
			break;	
		}
		default: break;
	}
}

Cyclogram::Iterator::Iterator(void *address):address(address) {}

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

