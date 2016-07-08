#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
 #include <stdint.h>
 #include <stddef.h>
 #include <stdbool.h>
 
 // id команд
 #define STOP  (0x7C6E)
 #define LOOP  (0x9FEE)
 #define START (0x28C8)
 #define PAUSE (0x7AAA)
 #define LDI   (0x9876)
 #define ADD   (0x5432)
 
 // Параметры команд
 #define END   (0xABCD)
 
 
struct IteratorAndCount;

struct Command {
	uint16_t num;
	uint16_t id;
	uint16_t time_s;
	uint16_t time_ms;
	uint16_t len;
	uint8_t data[];
	
	uint8_t* getCmdDataFromOffset(uint16_t offset);
	uint16_t get2BytesForm(uint8_t *source);
	void execute();
};


class Cyclogram {
	public:
		Cyclogram(void *base_address);
		
		void run(size_t cmdNo = 0);
	
		class Iterator {
			private:
				void *address;
			public: 
				Iterator(void *address);
				Command* operator *();
				Iterator& operator ++();
				//Iterator operator ++(int); // 
				Iterator& operator =(const Iterator &anotherIterator);
		};
	
	private:	
		void *base_address;
		
	public:
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
};


struct IteratorAndCount {
	Cyclogram::Iterator loopEntryIterator;
	uint16_t countOfIterations;
	IteratorAndCount(const Cyclogram::Iterator &loopEntryIterator, uint16_t countOfIterations);
	//IteratorAndCount& operator =(const IteratorAndCount &anotherIteratorAndCount);
};



#endif /* CYCLOGRAM_H_ */