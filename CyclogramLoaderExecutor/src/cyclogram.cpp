#include "avr/interrupt.h"
#include "uart_config.h"
#include "cyclogram.h"
#include "commands.h"
#include "timer.h"
#include <stddef.h>

/* Состояния выполнения циклограммы */
typedef enum CyclogramState : uint8_t {
	STATE_EMPTY,
	STATE_SAVE_DELAY_CONTEXT,
	STATE_RESTORE_DELAY_CONTEXT,
	STATE_EXT_CMD_PREPARE,
	STATE_EXT_CMD_DELAY, 
	STATE_EXT_CMD_EXECUTE,
	STATE_INT_CMD_PREPARE,
	STATE_INT_CMD_DELAY,
	STATE_INT_CMD_EXECUTE,
	STATE_CYCLOGRAM_PAUSE,
	STATE_CYCLOGRAM_STOP
} CyclogramState;

/* Текущее состояние циклограммы */
volatile static CyclogramState currCyclogramState = STATE_EMPTY;
/* Буфер состояния циклограммы, сохраняет состояние перед приходом внешней команды */
volatile static CyclogramState cyclogramStateBuf = STATE_EMPTY;

volatile static CyclogramState cyclogramStateBeforePause;

/* Значение счетчика миллисекунд в момент начала задержки выполнения команды */
volatile static uint32_t delayStart;
/* Число миллисекунд, на которое надо задержать выполнение команды */
volatile static uint32_t msecToDelay;

/* Буферный контекст задержки для восстановления после паузы */
volatile static uint32_t msecCountBuf;
volatile static uint32_t delayStartBuf;
volatile static uint32_t msecToDelayBuf;


uint8_t * Command::getCmdDataFromOffset(uint16_t offset) {
	return (this->data + offset);
}

uint16_t Command::get2BytesFrom(uint8_t *source) {
	return *((uint16_t *)source);
}

uint32_t Command::getMsecToDelay() {
	return (uint32_t)time_s*1000 + time_ms;
}

void Command::execute(Cyclogram *cyclogram) {
	/* Определение ID команды, в case описаны реализации предустановленных команд, в default - вызов реализации команды из внешнего файла */
	switch(id) {
		case CMD_ID_START: {
			Command *currCyclogramCmd = *(cyclogram->it);
			uint16_t currCyclogramCmdNum = currCyclogramCmd->num;
			uint16_t currCyclogramCmdId = currCyclogramCmd->id;
			uint16_t startCmdNum = get2BytesFrom(data);
			if(currCyclogramCmdNum != startCmdNum && currCyclogramCmdId != CMD_ID_STOP) {
				++(cyclogram->it);
			}
			else {
				currCyclogramState = STATE_INT_CMD_PREPARE;
			}
			uart_transmit_16(id);
			break;
		}
		case CMD_ID_STOP: {
			cyclogram->it.setTo(cyclogram->getBaseAddress());
			currCyclogramState = STATE_CYCLOGRAM_STOP;
			uart_transmit_16(id);
			break;
		}
		case CMD_ID_PAUSE: {
			if(cyclogramStateBuf != STATE_EMPTY) {
				cyclogramStateBeforePause = cyclogramStateBuf;
			}
			else {
				cyclogramStateBeforePause = currCyclogramState;
			}
			currCyclogramState = STATE_CYCLOGRAM_PAUSE;
			uart_transmit_16(id);
			break;
		}
		case CMD_ID_RESUME: {
			if(cyclogramStateBeforePause == STATE_INT_CMD_DELAY) {
				currCyclogramState = STATE_RESTORE_DELAY_CONTEXT;
			}
			else {
				currCyclogramState = cyclogramStateBeforePause;
			}
			uart_transmit_16(id);
			break;	
		}
		case CMD_ID_LOOP: {
			uint16_t currCmdFstParam = get2BytesFrom(data);
			switch(currCmdFstParam) {
				case CMD_PARAM_LOOP_START: {
					/* Чтобы при возврате на точку входа в цикл команда не заносилась в стек снова */
					Command *lastCmdOnStack = *(cyclogram->cmdStack.peek()->loopEntryIterator);
					if(this != lastCmdOnStack) {
						uint16_t countOfIterations = get2BytesFrom(getCmdDataFromOffset(CMD_OFFSET_LOOP_COUNT_OF_ITERATIONS));
						cyclogram->cmdStack.push(IteratorAndCount(cyclogram->it, (countOfIterations - 1)));
					}
					++(cyclogram->it);
					currCyclogramState = STATE_INT_CMD_PREPARE;
					uart_transmit_16(id);
					break;
				}
				case CMD_PARAM_LOOP_END: {
					IteratorAndCount *lastLoopEntry = cyclogram->cmdStack.peek();
					if(lastLoopEntry->countOfIterations > 0) {
						lastLoopEntry->countOfIterations--;
						cyclogram->it = lastLoopEntry->loopEntryIterator;
					}
					else {
						cyclogram->cmdStack.pop();
						++cyclogram->it;
					}
					currCyclogramState = STATE_INT_CMD_PREPARE;
					uart_transmit_16(id);
					break;
				}
				default: break;	
			}
			break;
		}
		default: {
			/* Выполнить команду из cmdsImp с индексом [id - COUNT_OF_PRESET_COMMANDS], передать в нее параметр (uint_8 *data) */
			(*(cyclogram->cmdsImp[id - COUNT_OF_PRESET_COMMANDS]))(data); 
			++(cyclogram->it);
			currCyclogramState = STATE_INT_CMD_PREPARE;
			break;
		}
	}
}

IteratorAndCount::IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations):
	loopEntryIterator(loopEntryIterator), 
	countOfIterations(countOfIterations) 
	{}

Cyclogram::Cyclogram(void *baseAddress, CmdImplementation *cmdsImp):
	baseAddress(baseAddress),
	it(baseAddress), 
	cmdStack((void *)CMD_STACK_BASE_ADDRESS, CMD_STACK_CAPACITY),
	cmdsImp(cmdsImp)
	{}

void Cyclogram::run() {
	/* Сначала проверяем, пришла ли внешняя команда */
	if(extCmdIsReceived) {
		/* Если внешняя команда прервала задержку внутренней команды - сохраняем контекст этой задержки */
		extCmdIsReceived = false;
		cyclogramStateBuf = currCyclogramState;
		if(currCyclogramState == STATE_INT_CMD_DELAY) {
			currCyclogramState = STATE_SAVE_DELAY_CONTEXT;
		}
		else {
			currCyclogramState = STATE_EXT_CMD_PREPARE;
		}
	}
	/* Проверка текущего состояния циклограммы */
	switch(currCyclogramState) {
		case STATE_SAVE_DELAY_CONTEXT: {
			msecCountBuf = msecCount;
			delayStartBuf = delayStart;
			msecToDelayBuf = msecToDelay;
			currCyclogramState = STATE_EXT_CMD_PREPARE;
			break;
		}
		case STATE_EXT_CMD_PREPARE: {
			extCmd = (Command *)EXT_CMD_BASE_ADDRESS;
			extCmdWordPtr = (uint16_t *)EXT_CMD_BASE_ADDRESS;
			msecToDelay = ((Command *)extCmd)->getMsecToDelay();
			delayStart = msecCount;
			currCyclogramState = STATE_EXT_CMD_DELAY;
			break;
		}
		case STATE_EXT_CMD_DELAY: {
			if((msecCount - delayStart) >= msecToDelay) {
				currCyclogramState = STATE_EXT_CMD_EXECUTE;
			}
			break;	
		}
		case STATE_EXT_CMD_EXECUTE: {
			/* Cмена состояния внутри execute() */
			Command *currCmd = (Command *)extCmd;
			currCmd->execute(this);
			break;	
		}
		case STATE_RESTORE_DELAY_CONTEXT: {
			msecCount = msecCountBuf;
			delayStart = delayStartBuf;
			msecToDelay = msecToDelayBuf;	
			currCyclogramState = cyclogramStateBeforePause;
			break;	
		}
		case STATE_INT_CMD_PREPARE: {
			Command *currCmd = *it;
			msecToDelay = currCmd->getMsecToDelay();
			delayStart = msecCount;
			currCyclogramState = STATE_INT_CMD_DELAY;
			break;
		}
		case STATE_INT_CMD_DELAY: {
			if((msecCount - delayStart) >= msecToDelay) {
				currCyclogramState = STATE_INT_CMD_EXECUTE;
			}
			break;
		}
		case STATE_INT_CMD_EXECUTE: {
			/* Инкремент итератора и смена состояния внутри execute() */
			Command *currCmd = *it;
			currCmd->execute(this);
			break;
		}
		default: break;
	}

}

void* Cyclogram::getBaseAddress() {
	return baseAddress;	
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


Cyclogram::Iterator& Cyclogram::Iterator::setTo(void *address) {
	this->address = address;
	return *this;
}

Cyclogram::CmdStack::CmdStack(void *base, size_t capacity) {
	this->base = (IteratorAndCount *)base;
	currElement = this->base;
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
		*(currElement--) = newElement;
		size++;
	}
}

IteratorAndCount* Cyclogram::CmdStack::pop() {
	if(!(this->isEmpty())) {
		size--;
		return ++currElement;
	}
	else {
		return nullptr;
	}
}

IteratorAndCount* Cyclogram::CmdStack::peek() {
	if(!(this->isEmpty())) {
		IteratorAndCount *tmp = currElement;
		return ++tmp;
	}
	else { 
		return nullptr;
	}
}

