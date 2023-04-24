#pragma once

#include "cantus.h"

#define DEBUG_UART 0
#define COM_UART    1

typedef struct{
	unsigned short *command;
	unsigned short *length;
	unsigned char *data;
	unsigned short *checksum;
}PROTOCOL_STRUCT;
PROTOCOL_STRUCT protocol;

#pragma pack(1)
typedef struct{
	unsigned char stx[8];
	unsigned short command;
	unsigned short length;
	unsigned short checksum;
	unsigned char etx[8];
}COM_ACK_STRUCT;
COM_ACK_STRUCT ack;
typedef struct{
	short voltage[33];
}COM_ACK_VOLT;
#pragma pack(4)

void uart1_isr(void);
void uart_task(void);
