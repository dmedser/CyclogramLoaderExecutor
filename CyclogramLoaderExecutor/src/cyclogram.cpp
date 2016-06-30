#include "cyclogram.h"
#include <stddef.h>

Cyclogram::Cyclogram(void* base_address) {
	this->base_address = base_address;
}

Cyclogram::Iterator::Iterator(void *address) {
	this->address = address;
}

Command* Cyclogram::Iterator::operator *() {
	return (Command*)(address);
}

Cyclogram::Iterator& Cyclogram::Iterator::operator ++() {
	this->address = (uint8_t *)(this->address) + (offsetof(Command, data) + (*(*this))->len);
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