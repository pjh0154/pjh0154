#include "ymodem.h"

unsigned char aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
char aFileName[FILE_NAME_LENGTH];
unsigned char file_size[64];
extern char fw_dn_flag;
extern long file_size_count;
extern int size_flag;
extern int sector_size;
extern char fpga_dw_flag;

unsigned char Ymodem_Receive(unsigned int *p_size)
{
	unsigned int i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
	unsigned int flashdestination;
	unsigned	int flash_destination = 0;	
	unsigned char *file_ptr;
	unsigned char *ramsource;
	//unsigned char file_size[16];
	unsigned int packets_received = 0;
	COM_StatusTypeDef result = COM_OK;
	unsigned char temp;
	
	if(sector_size == 0x8000)		
	{									
		flashdestination = 0x00008000;					// Cantus512A sector 32Kbyte
	}
	else
	{
		flashdestination = 0x00010000;					// Cantus512 sector 64Kbyt		
	}		
		
	if(fpga_dw_flag)											//Flash STAT_REG INIT
	{
		spi_stat_write(1,0x00);
		spi_stat_write(2,0x00);
		spi_stat_write(3,0x00);	
	}
	while ((session_done == 0) && (result == COM_OK))
	{
		packets_received = 0;
		file_done = 0;
		while ((file_done == 0) && (result == COM_OK))
		{
				temp=ReceivePacket(aPacketData, &packet_length);
				switch (temp)
				{
					case HAL_OK :
						errors = 0;
						switch (packet_length)
						{
							case 2:
								{
								Serial_PutByte(ACK);
								result = COM_ABORT;		
								break;
								}
							case 0:
								{
									Serial_PutByte(ACK);
									file_done = 1;
									break;
								}
							default :
									//if (aPacketData[PACKET_NUMBER_INDEX] != packets_received)
 									if (aPacketData[PACKET_NUMBER_INDEX] != (packets_received & 0x000000FF))								
									{
										Serial_PutByte(NAK);
									}
									else 
									{
										if (packets_received == 0)
										{
											if (aPacketData[PACKET_DATA_INDEX] != 0)
											{
												i = 0;
												file_ptr = aPacketData + PACKET_DATA_INDEX;			
												while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH))
												{
													aFileName[i++] = *file_ptr++;
												}									
												/* File size extraction */
												aFileName[i++] = '\0';
												i = 0;
												file_ptr ++;
												while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
												{
													file_size[i++] = *file_ptr++;
												}
												file_size[i++] = '\0';
												//Str2Int(file_size);
												file_size_count = Str2Int(file_size);
												if(fpga_dw_flag)
												{
													erase_block((unsigned int)FLASH_MEMORY_BLOCK0);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK1);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK2);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK3);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK4);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK5);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK6);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK7);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK8);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK9);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK10);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK11);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK12);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK13);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK14);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK15);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK16);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK17);
													erase_block((unsigned int)FLASH_MEMORY_BLOCK18);															
												}
												else
												{
													if(sector_size == 0x8000)					// Cantus512A sector 32Kbyte	
													{
														size_flag = file_size_count / (1024*32);
														norflash_erase(0x00008000, 32*1024*(size_flag + 1));
													}
													else													// Cantus512 sector 64Kbyte
													{
														size_flag = file_size_count / (1024*64);
														norflash_erase(0x00010000, 64*1024*(size_flag + 1));													
													}
												}
												Serial_PutByte(ACK);
												Serial_PutByte(CRC16);
											}
											else
											{
												Serial_PutByte(ACK);
												file_done = 1;
												session_done = 1;
												break;								
											}
										}
										else /* data packet */
										{
											if(fpga_dw_flag)
											{
												ramsource = (unsigned char *) &aPacketData[PACKET_DATA_INDEX];

												spi_write(PAGE_PROGREAM,(unsigned int)FLASH_MEMORY_BLOCK0+flash_destination,ramsource,256);
												spi_write(PAGE_PROGREAM,(unsigned int)FLASH_MEMORY_BLOCK0+flash_destination+256,ramsource+256,256);
												spi_write(PAGE_PROGREAM,(unsigned int)FLASH_MEMORY_BLOCK0+flash_destination+(256*2),ramsource+(256*2),256);
												spi_write(PAGE_PROGREAM,(unsigned int)FLASH_MEMORY_BLOCK0+flash_destination+(256*3),ramsource+(256*3),256);
												flash_destination += packet_length;
												Serial_PutByte(ACK);												
											}
											else
											{
												ramsource = (unsigned char *) &aPacketData[PACKET_DATA_INDEX];
												if(norflash_write( ramsource, flashdestination, packet_length) == 0)
												{
													flashdestination += packet_length;
													Serial_PutByte(ACK);
												}
											
												else  /* An error occurred while writing to Flash memory */
												{
													/* End session */
													Serial_PutByte(CA);
													Serial_PutByte(CA);
													result = COM_DATA;							
												}
											}
										}
									packets_received ++;
									session_begin = 1;	
									}					
								break;
						}
						break;
					case HAL_BUSY :
						Serial_PutByte(CA);
						Serial_PutByte(CA);
						
						result = COM_ABORT;
						break;
					default :
						if (session_begin > 0)
						{
							errors ++;
						}
						if (errors > MAX_ERRORS)
						{
						/* Abort communication */
							Serial_PutByte(CA);
							Serial_PutByte(CA);
						}
						else
						{
							Serial_PutByte(CRC16);  /* Ask for a packet */
						}
						break;
				}
		}
	}
	return result;
	
}


static unsigned char ReceivePacket(unsigned char *p_data, unsigned int *p_length)
{
	unsigned short crc_check;
	unsigned int recv_cnt = 0;
	unsigned int count = 1000000;  /* TIME OUT 1sec*/
	unsigned short crc;
	unsigned int packet_size = 0;
	
	HAL_StatusTypeDef status = HAL_ERROR;
	HAL_StatusTypeDef result;
	unsigned char char1 = 0;
	*p_length = 0;
	if(!fw_dn_flag)
	{
		while(count--)
		{
			if(UartGetCh(DEBUG_UART_PORT, &char1) != 0) 
			{
				status =1;
				break;
			}
				asm("nop");
		}
	}
	else 
	{
		while(count--)
		{
			if(UartGetCh(UART_SUB_HANDLE, &char1) != 0) 
			{
				status =1;
				break;
			}
				asm("nop");
		}	
	}
	if(status == HAL_OK)
	{
		switch(char1)
		{
		case SOH:	
		packet_size = PACKET_SIZE;
		break;
		case STX:
		packet_size = PACKET_1K_SIZE;
		break;
		case EOT:
		break;
		case CA:
			if(!fw_dn_flag)
			{
				while(count--)
				{
					if(UartGetCh(DEBUG_UART_PORT, &char1) != 0)
					{
						result = HAL_OK;
						break;
					}
					asm("nop");
				}			
				if ((result == HAL_OK) && (char1 == CA))
				{
					packet_size = 2;
				}
				else
				{
					status = HAL_ERROR;
				}
			}
			else
			{
				while(count--)
				{
					if(UartGetCh(UART_SUB_HANDLE, &char1) != 0)
					{
						result = HAL_OK;
						break;
					}
					asm("nop");
				}			
				if ((result == HAL_OK) && (char1 == CA))
				{
					packet_size = 2;
				}
				else
				{
					status = HAL_ERROR;
				}			
			}
				break;
		case ABORT1 :
		case ABORT2 :
			status = HAL_BUSY;
			break;
		default:
			status = HAL_ERROR;
			break;
		}
		
		*p_data = char1;
		
		if (packet_size >= PACKET_SIZE )
		{
			recv_cnt = 0;
			if(!fw_dn_flag)
			{			
				while(count--)
				{
					if(UartGetCh(DEBUG_UART_PORT, &char1) == HAL_OK)
					{
						aPacketData[recv_cnt+2]  = (char)char1;
						recv_cnt++;
					}
					if(recv_cnt >= packet_size + PACKET_OVERHEAD_SIZE)
					{
						status = HAL_OK;
						break;
					}
					status = HAL_ERROR;
					asm("nop");
				}
			}
			else 
			{
				while(count--)
				{
					if(UartGetCh(UART_SUB_HANDLE, &char1) == HAL_OK)
					{
						aPacketData[recv_cnt+2]  = (char)char1;
						recv_cnt++;
					}
					if(recv_cnt >= packet_size + PACKET_OVERHEAD_SIZE)
					{
						status = HAL_OK;
						break;
					}
					status = HAL_ERROR;
					asm("nop");
				}			
			}
			
			if(status == HAL_OK)
			{
				if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
				{	
				  packet_size = 0;
				  status = HAL_ERROR;
				}
				else 
				{
				  /* Check packet CRC */
				  crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
				  crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];

				  
				  crc_check = Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size);
				  if(crc_check != crc)
				  {
						packet_size = 0;
						status = HAL_ERROR;
				  }
				}				
			}
			else
			{
				packet_size = 0;
			}
		}
	}
	*p_length = packet_size;
	return status;
}


unsigned short Cal_CRC16(const unsigned char* p_data, unsigned int size)
{
	unsigned int crc = 0;
	const unsigned char* dataEnd = p_data+size;

	while(p_data < dataEnd) crc = UpdateCRC16(crc, *p_data++);

	crc = UpdateCRC16(crc, 0);
	crc = UpdateCRC16(crc, 0);

	return crc&0xffff;	
}

unsigned short  UpdateCRC16(unsigned short crc_in, unsigned char byte)
{
	unsigned int crc = crc_in;
	unsigned int in = byte | 0x100;

	do
	{
		crc <<= 1;
		in <<= 1;
		if(in & 0x100)
		  ++crc;
		if(crc & 0x10000)
		  crc ^= 0x1021;
	}

	while(!(in & 0x10000));

	return crc & 0xffffu;
}

static unsigned long Str2Int(unsigned char* str)
{
	const char *s = str;
	unsigned long acc;
	int c;

	/* strip leading spaces if any */
	do {   
		c = *s++;
	} while (c == ' ');

	for (acc = 0; (c >= '0') && (c <= '9'); c = *s++) 
	{
		c -= '0';
		acc *= 10;
		acc += c;
	}
	return acc;
}

HAL_StatusTypeDef Serial_PutByte(unsigned char param)
{
	unsigned int count = 100000;
	HAL_StatusTypeDef result = HAL_ERROR;
	if(!fw_dn_flag)
	{
		while(count--)
		{
			if(UartPutCh(DEBUG_UART_PORT, param)!=0) 
			{
				result = HAL_OK;
				break;
			}
			asm("nop");
		}				
	}
	else
	{
		while(count--)
		{
			if(UartPutCh(UART_SUB_HANDLE, param)!=0) 
			{
				result = HAL_OK;
				break;
			}
			asm("nop");
		}	
	}
	return result;
}
