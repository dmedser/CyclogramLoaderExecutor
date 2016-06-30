#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
 #include <stdint.h>
 #include <stddef.h>
 
 

struct Command {
	uint16_t num;
	uint16_t id;
	uint16_t time_s;
	uint16_t time_ms;
	uint16_t len;
	uint8_t data[];
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
				Iterator(const Iterator &anotherIterator);
				Command* operator*();
				Iterator& operator++();
				Iterator operator++(int);
				Command* getCurrCmdAddress();
		};
		
		class CmdStack {
			public:
				CmdStack(void *base, size_t count);
				void push(Command *address);
				Command* pop();
				Command* peek();
			private:
				Command **base;
				size_t capacity;
				size_t size;
				Command **curr_address;
		};
		
	private:
		void *base_address;
};



#endif /* CYCLOGRAM_H_ */