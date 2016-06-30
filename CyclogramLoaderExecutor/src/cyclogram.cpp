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
	this->address += (offsetof(Command, data) + (*(*this))->len);
}

void* Cyclogram::Iterator::getCurrAddress() {
	return address;
}

Cyclogram::CmdStack::CmdStack(void *base, void *end) {
	this->base = base;
	this->end = end;
	curr_address = (uint16_t *)base;
}

void Cyclogram::CmdStack::push(void *address) {
	if(curr_address == (uint16_t *)base) {
		*curr_address = (uint16_t)address;
	}
	else if (curr_address != (uint16_t *)end) {
		*(++curr_address) = (uint16_t)address;		
	}
}

void* Cyclogram::CmdStack::pop() {
	if(curr_address == (uint16_t *)base) {
		return (void *)(*curr_address);
	}
	else {
		return (void *)(*(curr_address--));
	} 
}

void* Cyclogram::CmdStack::peek() {
	return (void *)(*curr_address);
}