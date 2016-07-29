#ifndef CMDS_H_
#define CMDS_H_
#include <stdint.h>

#define COMMANDS_NUMBER (4)

void ldi(uint8_t *);
void add(uint8_t *);
void sbi(uint8_t *);
void cbi(uint8_t *);


#endif /* CMDS_H_ */