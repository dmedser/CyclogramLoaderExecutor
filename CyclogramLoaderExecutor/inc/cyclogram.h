#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Для работы с ядром необходимо включить в проект дополнительные файлы:
 *	- таймер, реализующий инкремент переменной uint32_t msecCount раз в 1 мс
 *	- интерфейс передачи данных, реализующий загрузку внешней команды, с использованием сл. переменных: 
 *		- uint16_t *extCmdWordPtr - указатель на область памяти, которая хранит внешнюю команду
 *		- bool extCmdIsReceived 
 *		- Command *extCmd 
 *	- .cpp файл с реализациями команд (команды имеют вид: void cmdName(uint8_t *)) 
 */
 
/* Адрес первой команды циклограммы */ 
#define CYCLOGRAM_BASE_ADDRESS		0x3F0

/* ID предустановленных команд */
#define CMD_ID_START				(0)
#define CMD_ID_STOP					(1)
#define CMD_ID_PAUSE				(2)
#define CMD_ID_RESUME				(3)
#define CMD_ID_LOOP					(4)

/* Параметры предустановленной команды LOOP */
#define CMD_PARAM_LOOP_START		(0x28C8)
#define CMD_PARAM_LOOP_END			(0xABCD)
 
/* Стек команд служит для работы предустановленной команды LOOP, см. класс Cyclogram */ 
#define CMD_STACK_BASE_ADDRESS		(0x15DC)
#define CMD_STACK_CAPACITY			(2)

/* Адрес хранения внешней команды */
#define EXT_CMD_BASE_ADDRESS		(0x02F8)

#define COUNT_OF_PRESET_COMMANDS	(5)

/* Отступ в байтах числа итераций для команды LOOP */
#define CMD_OFFSET_LOOP_COUNT_OF_ITERATIONS		(2)

/* Тип "реализация команды" - указатель на void функцию с параметром uint8_t* */ 
typedef void (*CmdImplementation)(uint8_t *);

struct IteratorAndCount;
class Cyclogram;

struct Command {
	uint16_t num;
	uint16_t id;
	uint16_t time_s;
	uint16_t time_ms;
	uint16_t len;
	uint8_t data[];
	
	/* Служебные методы для работы с полями команды */
	uint8_t* getCmdDataFromOffset(uint16_t offset);
	uint16_t get2BytesFrom(uint8_t *source);
	uint32_t getMsecToDelay();
	
	/*
	 * Команде может потребоваться доступ к стеку команд или итератору, 
	 * поэтому передаем указатель на циклограмму 
	 */
	void execute(Cyclogram *cyclogram);
};


class Cyclogram {
	friend struct IteratorAndCount;
	friend struct Command; 
	public:
		Cyclogram(void *baseAddress, CmdImplementation *cmdsImp);
		void run();
		void* getBaseAddress();
		
	private:
		/* Стек команд хранит пары <точка входа в цикл, число итераций>, см. структуру IteratorAndCount */
		class CmdStack {
			public:
				CmdStack(void *base, size_t capacity);
				bool isEmpty();
				bool isFull();
				void push(const IteratorAndCount &newElement);
				IteratorAndCount* pop();
				IteratorAndCount* peek();
			private:
				IteratorAndCount *base;
				size_t capacity;
				size_t size;
				IteratorAndCount *currElement;
		};
		class Iterator {
			private:
				void *address;
			public:
				Iterator(void *address);
				Command* operator *();
				Iterator& operator ++();
				Iterator& operator =(const Iterator &anotherIterator);
				Iterator& setTo(void *address);
		};
		
		void *baseAddress;
		Iterator it;
		CmdStack cmdStack;
		/* Указатель на первый элемент массива из void функций с аргументом uint8_t* - реализаций команд */
		CmdImplementation *cmdsImp;
};

/* Содержимое стека команд */
struct IteratorAndCount {
	/* Итератор, указывающий на команду - точку входа в цикл */
	Cyclogram::Iterator loopEntryIterator;
	uint16_t countOfIterations;
	IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations);
};

#endif /* CYCLOGRAM_H_ */