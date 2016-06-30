#ifndef CYCLOGRAM_H_
#define CYCLOGRAM_H_
 #include <stdint.h>
 
 

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
		
		class Iterator {
			private:
				void *address;
			public:
				Iterator(void *address);
				Command* operator*();
				Iterator& operator++();
				void* getCurrAddress();
		};
		
		class CmdStack {
			public:
				CmdStack(void *base, void *end);
				void push(void *address);
				void* pop();
				void* peek();
			private:
				void *base;
				void *end;
				uint16_t *curr_address;
		};
		
	private:
		void *base_address;
};



#endif /* CYCLOGRAM_H_ */