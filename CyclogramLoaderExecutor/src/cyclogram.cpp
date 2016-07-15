#include "avr/interrupt.h"
#include "uart_config.h"
#include "cyclogram.h"
#include "timer.h"
#include <util/delay.h>
#include <stddef.h>

#define STACK_CAPACITY						(2)
#define STACK_BASE							(0x15E8)

#define LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET	(2) 
#define LDI_CMD_VALUE_OFFSET				(1)
#define ADD_CMD_SECOND_REG_OFFSET			(1)
#define BIT_CMD_BIT_NUMBER_OFFSET			(1)

volatile static uint8_t byteCounter = 0;
volatile static uint16_t extCmd = 0;

ISR(USART1_RX_vect) {
	byteCounter++;
	if(byteCounter < sizeof(uint16_t)) {
		extCmd = UDR1;
	}
	else {
		extCmd <<= 8;
		extCmd |= UDR1;
		byteCounter = 0;
	}
}

volatile static uint16_t tickCount = 0;


ISR(TIMER3_OVF_vect) {
	tickCount++;
}

uint8_t * Command::getCmdDataFromOffset(uint16_t offset) {
	return (this->data + offset);
}

uint16_t Command::get2BytesForm(uint8_t *source) {
	return *((uint16_t *)source);
}

void Command::execute() {
	volatile uint16_t fractPart = TCNT3;
	volatile uint16_t integerPart = tickCount;
	count_to(time_s, time_ms);
	uart_transmit_16(HEADER);
	uart_transmit_16(num);
	uart_transmit_16(id); 
	uart_transmit_16(integerPart);
	uart_transmit_16(fractPart); // fractional part
	
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
			uint8_t bitNo = *getCmdDataFromOffset(BIT_CMD_BIT_NUMBER_OFFSET);
			switch(portName) {
				case 'D': {DDRD = 0xFF; PORTD |= (1 << bitNo); break;}
				case 'F': {DDRF = 0xFF; PORTF |= (1 << bitNo); break;}
				default: break;
			}
			break;
			
		} 
		case CBI: {
			char portName = (char)*data;
			uint8_t bitNo = *getCmdDataFromOffset(BIT_CMD_BIT_NUMBER_OFFSET);
			switch(portName) {
				case 'D': {DDRD = 0xFF; PORTD &= ~(1 << bitNo); break;}
				case 'F': {DDRF = 0xFF; PORTF &= ~(1 << bitNo); break;}
				default: break;
			}
			break;
		}
		default: break;
	}
	
	
	
	volatile uint8_t o = PORTB;	//
}

IteratorAndCount::IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations):loopEntryIterator(loopEntryIterator), countOfIterations(countOfIterations) {}

Cyclogram::Cyclogram(void* base_address):base_address(base_address) {}

void Cyclogram::run(size_t cmdNo) {
	//while(extCmd != START);
	
 	Iterator it(base_address);
	if(cmdNo != 0) {
		while((*it)->num != cmdNo) {
			++it;
		}
	}
	
	CmdStack cmdStack((void *)STACK_BASE, STACK_CAPACITY);
	
	while((*it)->id != STOP) {
		
		if(extCmd == STOP) {
			break;
		}
		if(extCmd == PAUSE) {
			while(extCmd != START);
		}
		
		Command *currCmd = *it;
		uint16_t currCmdId = currCmd->id; 
		uint16_t currCmdFstParam = currCmd->get2BytesForm(currCmd->data);
		
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
				currCmd->execute();		//
				continue;				//
 			}
			else {
				cmdStack.pop();		
			}
		}
		currCmd->execute();	
		++it;	
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

