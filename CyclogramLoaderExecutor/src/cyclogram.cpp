#include "cyclogram.h"
#include <stddef.h>
#include "uart_config.h"
#include <util/delay.h>

#define STOP  (0x7C6E)
#define LOOP  (0x9FEE)
#define START (0x28C8)
#define END   (0xABCD)

#define STACK_CAPACITY (10)

#define STACK_BASE (0x5E8)

Cyclogram::Cyclogram(void* base_address) {
	this->base_address = base_address;
}


void Cyclogram::run(size_t cmdNo) {
	Iterator it(base_address);
	if(cmdNo != 0) {
		while((*it)->num != cmdNo) {
			++it;
		}
	}
	
	Cyclogram::CmdStack cmdStack((void *)STACK_BASE, STACK_CAPACITY);

	while((*it)->id != STOP) {
		uart_transmit_16((*it)->num);
		it++;	
	}
}


Cyclogram::Iterator::Iterator(void *address) {
	this->address = address;
}

Cyclogram::Iterator::Iterator(const Iterator &anotherIterator) {
	this->address = anotherIterator.address;	
}

Command* Cyclogram::Iterator::operator *() {
	return (Command*)(address);
}

Cyclogram::Iterator& Cyclogram::Iterator::operator ++() {
	this->address = (uint8_t *)(this->address) + (offsetof(Command, data) + (*(*this))->len);
	return *this;
}

Cyclogram::Iterator Cyclogram::Iterator::operator ++(int) {
	Iterator tmp(*this);
	++(*this);
	return tmp;
}

Command* Cyclogram::Iterator::getCurrCmdAddress() {
	return (Command *)address;
}

Cyclogram::CmdStack::CmdStack(void *base, size_t count) {
	this->base = (Command **)base;
	curr_address = this->base;
	capacity = count;
	size = 0;
}

void Cyclogram::CmdStack::push(Command *address) {
	if (size < capacity) {
		*(curr_address++) = address;
		size++;
	}
}

Command* Cyclogram::CmdStack::pop() {
	if(size > 0) {
		size--;
		return *(--curr_address);
	}
	else {
		return nullptr;
	}
}

Command* Cyclogram::CmdStack::peek() {
	if(size > 0) {
		Command **tmp = curr_address;
		return *(--tmp);
	}
	else { 
		return nullptr;
	}
}