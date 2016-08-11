#ifndef CMDS_H_
#define CMDS_H_
#include <stdint.h>

/* CMD_ID � 0 �� 4  - id ����������������� ������ */

#define CMD_ID_LDI		(5)
#define CMD_ID_ADD		(6)
#define CMD_ID_SBI		(7)
#define CMD_ID_CBI		(8)

/* ������� � ������ ��� ���������� ��������� ������ */
#define CMD_OFFSET_LDI_VALUE			(1)
#define CMD_OFFSET_ADD_SECOND_REG		(1)
#define CMD_OFFSET_BITWISE_BIT_NUMBER	(1)

#define COMMANDS_NUMBER (4)

void ldi(uint8_t *);
void add(uint8_t *);
void sbi(uint8_t *);
void cbi(uint8_t *);


#endif /* CMDS_H_ */