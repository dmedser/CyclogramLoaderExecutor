#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
 #include <stdint.h>
 #include <stddef.h>
 #include <stdbool.h>

 #define ID_START			(0)
 #define ID_STOP			(1)
 #define ID_PAUSE			(2)
 #define ID_LOOP			(3)
 #define ID_LDI				(4)
 #define ID_ADD				(5)
 #define ID_SBI				(6)
 #define ID_CBI				(7)
 #define PARAM_LOOP_START	(0x28C8)
 #define PARAM_LOOP_END		(0xABCD)
 
 #define HEADER (0x7C6E)
 
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
	
	uint8_t* getCmdDataFromOffset(uint16_t offset);
	uint16_t get2BytesForm(uint8_t *source);
	void execute(Cyclogram *cyclogram);
};


class Cyclogram {
	friend struct IteratorAndCount;
	friend struct Command; 
	public:
		Cyclogram(void *baseAddress, CmdImplementation *cmdsImp);
		void run();
		
	private:	
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
				IteratorAndCount *curr_element;
		};
		
		class Iterator {
			private:
				void *address;
			public:
				Iterator(void *address);
				Command* operator *();
				Iterator& operator ++();
				Iterator& operator =(const Iterator &anotherIterator);
		};
		
		void *baseAddress;
		Iterator it;
		CmdStack cmdStack;
		/* ”казатель на первый элемент массива из void функций с аргументом uint8_t* */
		CmdImplementation *cmdsImp;
};


struct IteratorAndCount {
	Cyclogram::Iterator loopEntryIterator;
	uint16_t countOfIterations;
	IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations);
};

#endif /* CYCLOGRAM_H_ */