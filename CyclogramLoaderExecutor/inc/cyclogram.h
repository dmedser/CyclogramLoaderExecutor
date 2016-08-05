#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
 #include <stdint.h>
 #include <stddef.h>
 #include <stdbool.h>

 #define ID_START			(0)
 #define ID_STOP			(1)
 #define ID_PAUSE			(2)
 #define ID_RESUME			(3)
 #define ID_LOOP			(4)
 #define ID_LDI				(5)
 #define ID_ADD				(6)
 #define ID_SBI				(7)
 #define ID_CBI				(8)
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
	uint16_t get2BytesFrom(uint8_t *source);
	uint32_t getMsecToDelay();
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
		/* ”казатель на первый элемент массива из void функций с аргументом uint8_t* */
		CmdImplementation *cmdsImp;
};


struct IteratorAndCount {
	Cyclogram::Iterator loopEntryIterator;
	uint16_t countOfIterations;
	IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations);
};

#endif /* CYCLOGRAM_H_ */