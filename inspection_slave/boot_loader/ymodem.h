#include "cantus.h"
#include "ep.h"

unsigned char  Ymodem_Receive(unsigned int *p_size);
static unsigned char ReceivePacket(unsigned char *p_data, unsigned int *p_length);
unsigned short Cal_CRC16(const unsigned char* p_data, unsigned int size);
unsigned short  UpdateCRC16(unsigned short crc_in, unsigned char byte);
static unsigned long Str2Int(unsigned char* str);
HAL_StatusTypeDef Serial_PutByte(unsigned char param);
