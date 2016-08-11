#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * ��� ������ � ����� ���������� �������� � ������ �������������� �����:
 *	- ������, ����������� ��������� ���������� uint32_t msecCount ��� � 1 ��
 *	- ��������� �������� ������, ����������� �������� ������� �������, � �������������� ��. ����������: 
 *		- uint16_t *extCmdWordPtr - ��������� �� ������� ������, ������� ������ ������� �������
 *		- bool extCmdIsReceived 
 *		- Command *extCmd 
 *	- .cpp ���� � ������������ ������ (������� ����� ���: void cmdName(uint8_t *)) 
 */
 
/* ����� ������ ������� ����������� */ 
#define CYCLOGRAM_BASE_ADDRESS		0x3F0

/* ID ����������������� ������ */
#define CMD_ID_START				(0)
#define CMD_ID_STOP					(1)
#define CMD_ID_PAUSE				(2)
#define CMD_ID_RESUME				(3)
#define CMD_ID_LOOP					(4)

/* ��������� ����������������� ������� LOOP */
#define CMD_PARAM_LOOP_START		(0x28C8)
#define CMD_PARAM_LOOP_END			(0xABCD)
 
/* ���� ������ ������ ��� ������ ����������������� ������� LOOP, ��. ����� Cyclogram */ 
#define CMD_STACK_BASE_ADDRESS		(0x15DC)
#define CMD_STACK_CAPACITY			(2)

/* ����� �������� ������� ������� */
#define EXT_CMD_BASE_ADDRESS		(0x02F8)

#define COUNT_OF_PRESET_COMMANDS	(5)

/* ������ � ������ ����� �������� ��� ������� LOOP */
#define CMD_OFFSET_LOOP_COUNT_OF_ITERATIONS		(2)

/* ��� "���������� �������" - ��������� �� void ������� � ���������� uint8_t* */ 
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
	
	/* ��������� ������ ��� ������ � ������ ������� */
	uint8_t* getCmdDataFromOffset(uint16_t offset);
	uint16_t get2BytesFrom(uint8_t *source);
	uint32_t getMsecToDelay();
	
	/*
	 * ������� ����� ������������� ������ � ����� ������ ��� ���������, 
	 * ������� �������� ��������� �� ����������� 
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
		/* ���� ������ ������ ���� <����� ����� � ����, ����� ��������>, ��. ��������� IteratorAndCount */
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
		/* ��������� �� ������ ������� ������� �� void ������� � ���������� uint8_t* - ���������� ������ */
		CmdImplementation *cmdsImp;
};

/* ���������� ����� ������ */
struct IteratorAndCount {
	/* ��������, ����������� �� ������� - ����� ����� � ���� */
	Cyclogram::Iterator loopEntryIterator;
	uint16_t countOfIterations;
	IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations);
};

#endif /* CYCLOGRAM_H_ */