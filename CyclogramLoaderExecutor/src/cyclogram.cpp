#include "cyclogram.h"
#include <stddef.h>
#include "uart_config.h"
#include <util/delay.h>
#include "timer.h"

#define STOP  (0x7C6E)
#define LOOP  (0x9FEE)
#define START (0x28C8)
#define END   (0xABCD)

#define STACK_CAPACITY	(2)
#define STACK_BASE		(0x5E8)
#define LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET	(2) 


uint8_t * Command::getCmdDataFromOffset(uint16_t offset) {
	return (this->data + offset);
}

uint16_t Command::get2BytesForm(uint8_t *source) {
	return *((uint16_t *)source);
}

void Command::execute() {
	countTo(this->time_s, time_ms);
	uart_transmit_16(this->num);
}

IteratorAndCount::IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations):loopEntryIterator(loopEntryIterator), countOfIterations(countOfIterations) {}

IteratorAndCount& IteratorAndCount::operator =(const IteratorAndCount& anotherIteratorAndCount) {
	this->loopEntryIterator = anotherIteratorAndCount.loopEntryIterator;
	this->countOfIterations = anotherIteratorAndCount.countOfIterations;
	return *this;
}

Cyclogram::Cyclogram(void* base_address):base_address(base_address) {}

void Cyclogram::run(size_t cmdNo) {
	Iterator it(base_address);
	if(cmdNo != 0) {
		while((*it)->num != cmdNo) {
			++it;
		}
	}
	
	CmdStack cmdStack((void *)STACK_BASE, STACK_CAPACITY);
	
	while((*it)->id != STOP) {
		Command *currCmd = *it;
		  
		if(currCmd->id == LOOP && currCmd->get2BytesForm(currCmd->data) == START) {
			if(currCmd != *(cmdStack.peek()->loopEntryIterator)) {		// 
				uint16_t countOfIterations = currCmd->get2BytesForm(currCmd->getCmdDataFromOffset(LOOP_CMD_COUNT_OF_ITERATIONS_OFFSET));
				cmdStack.push(IteratorAndCount(it, (countOfIterations - 1)));
			}															//
		}
		if(currCmd->id == LOOP && currCmd->get2BytesForm(currCmd->data) == END) {
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

 	uint8_t o = 0;
	
}


Cyclogram::Iterator::Iterator(void *address):address(address) {}

Command* Cyclogram::Iterator::operator *() {
	return (Command*)(address);
}

Cyclogram::Iterator& Cyclogram::Iterator::operator ++() {
	this->address = (uint8_t *)(this->address) + (offsetof(Command, data) + (*(*this))->len);
	return *this;
}

Cyclogram::Iterator Cyclogram::Iterator::operator ++(int) {
	Iterator tmp = *this;
	++(*this);
	return tmp;
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