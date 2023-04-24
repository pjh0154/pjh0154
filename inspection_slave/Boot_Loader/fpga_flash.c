#include	"fpga_flash.h"
 w25q32_t w25q32;
unsigned char	packet_read[256] = {0};


void spi_stat_read(unsigned char cmd)
{
	unsigned char stat_num = 0;
	unsigned char	stat_read = 0;
	if(cmd == 1)			stat_num = READ_STAT_REGISTER_1;
	else if(cmd ==2)	stat_num = READ_STAT_REGISTER_2;
	else if(cmd ==3)	stat_num = READ_STAT_REGISTER_3;	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	byte_data_write(stat_num);
	SPI_SCLK_LOW;
	stat_read = byte_data_read();	
	SPI_nCS_HIGH;
	SPI_SCLK_HIGH;
	debugprintf("STAT%d = %x\r\n",cmd, stat_read );
}
void spi_stat_write(unsigned char cmd, unsigned char data)
{
	unsigned char stat_num = 0;
	if(cmd == 1)			stat_num = WRITE_STAT_REGISTER_1;
	else if(cmd ==2)	stat_num = WRITE_STAT_REGISTER_2;
	else if(cmd ==3)	stat_num = WRITE_STAT_REGISTER_3;
	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;

	SPI_nCS_LOW;
	byte_data_write(STAT_WRITE_ENABLE_F);
	SPI_nCS_HIGH;
	SPI_SCLK_HIGH;	
	SPI_nCS_LOW;
	byte_data_write(stat_num);
	byte_data_write(data);	
	SPI_nCS_HIGH;
	SPI_SCLK_HIGH;		
}
void spi_write(unsigned char cmd,unsigned int addr, unsigned char  *data,int len)
{
  	unsigned int  i;	
	unsigned char packet[1+3+len];
		
	packet[0]=cmd;
	packet[1]=(addr>>16)&0xFF;
	packet[2]=(addr>>8)&0xFF;
	packet[3]=addr&0xFF;	
	while (w25q32.Lock == 1)
	w25q32.Lock = 1;	
	memcpy(packet+4,data,len);
	wait_for_write_end();
	write_enable();		
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;

	for(i = 0 ; i<len+4 ; i++)
	{
		byte_data_write(packet[i]);
	}
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	wait_for_write_end();
	w25q32.Lock = 0;
	i=0;
}
void  spi_read(unsigned char cmd,unsigned int addr,int len)
{
	unsigned int  i,j;	
	unsigned char packet_w[5];
		
	packet_w[0]=cmd;
	packet_w[1]=(addr>>16)&0xFF;
	packet_w[2]=(addr>>8)&0xFF;
	packet_w[3]=addr&0xFF;	
	packet_w[4]=0x00;
	
	memset(&packet_read, 0, sizeof(packet_read));
	
	while (w25q32.Lock == 1)
	delayms(1);	
	w25q32.Lock = 1;	
	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
 	if(cmd == 0x0B)
	{
		for(i = 0 ; i < 5 ; i++)
		{
			byte_data_write(packet_w[i]);	
		}
	}
	else
	{
		for(i = 0 ; i < 4 ; i++)
		{
			byte_data_write(packet_w[i]);	
		}	
	} 
	SPI_SCLK_LOW;
 	for(j=0;j<len;j++)
	{		
		packet_read[j] = byte_data_read();
	} 
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	i = 0;
	j = 0;
	w25q32.Lock = 0;
}
void byte_data_write(unsigned char data)
{
	unsigned int  i;	
	SPI_SCLK_HIGH;
	for(i = 0x80 ; i > 0 ; i>>=1)
	{
		SPI_SCLK_LOW;
		if(data & i)	SPI_MOSI_HIGH;
		else				SPI_MOSI_LOW;	
		SPI_SCLK_HIGH;
	}
}
unsigned char byte_data_read()
{
	unsigned int  i;	
	unsigned char rev = 0;
	unsigned char rev_d = 0;	
	
	for(i = 0 ; i<8 ; i++)
	{
		SPI_SCLK_HIGH;
		rev = ((*R_P5iLEV >> 4) & 0x01);
		SPI_SCLK_LOW;
		rev_d = (rev_d<<0x01) | rev;
	}
	//debugprintf("STAT  = %x \r\n", rev_d);
	return rev_d;
}
void wait_for_write_end()
{
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	byte_data_write(READ_STAT_REGISTER_1);
	SPI_SCLK_LOW;
	do
	{
		w25q32.StatusRegister1 = byte_data_read();
	}while	((w25q32.StatusRegister1 & 0x01)	==	0x01);
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;	
} 
void write_enable()
{
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	byte_data_write(WRITE_ENABLE_F);
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
}
void write_disable()
{
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	byte_data_write(WRITE_DISABLE_F);
	SPI_SCLK_HIGH;	
	SPI_nCS_HIGH;
}
void flash_init()
{
	unsigned char stat_num = 0;
	int i;

	spi_stat_write(1,0x00);
	spi_stat_write(2,0x00);
	spi_stat_write(3,0x00);
	
	for(i=1 ; i<4 ; i++)
	{
		if(i == 1)			stat_num = READ_STAT_REGISTER_1;
		else if(i ==2)	stat_num = READ_STAT_REGISTER_2;
		else if(i ==3)	stat_num = READ_STAT_REGISTER_3;	
		SPI_SCLK_HIGH;
		SPI_nCS_HIGH;
		SPI_nCS_LOW;
		byte_data_write(stat_num);
		SPI_SCLK_LOW;
		if(stat_num == READ_STAT_REGISTER_1)	w25q32.StatusRegister1 = byte_data_read();	
		else if(stat_num == READ_STAT_REGISTER_2)	w25q32.StatusRegister2 = byte_data_read();	
		else if(stat_num == READ_STAT_REGISTER_3)	w25q32.StatusRegister3 = byte_data_read();	
		SPI_nCS_HIGH;
		SPI_SCLK_HIGH;	
	}
	debugprintf("StatusRegister1 = %x / StatusRegister2 = %x / StatusRegister3 = %x\r\n", w25q32.StatusRegister1,w25q32.StatusRegister2,w25q32.StatusRegister3);
}
void erase_block(unsigned int blockaddr)
{
	unsigned char packet[4] = {0,};
	int i;
	packet[0] = BLOCK_ERASE_64K;
	while (w25q32.Lock == 1)
	delayms(1);	
	w25q32.Lock = 1;
	delayms(100);
	wait_for_write_end();
	write_enable();
	wait_for_write_end();	
	packet[1]=(blockaddr>>16)&0xFF;
	packet[2]=(blockaddr>>8)&0xFF;
	packet[3]=blockaddr&0xFF;	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	delayms(1);
 	for(i = 0 ; i < 4 ; i++)
	{
		byte_data_write(packet[i]);	
	}   
	
	SPI_SCLK_HIGH;
	delayms(1);
	SPI_nCS_HIGH;	
	wait_for_write_end();
	delayms(100);
	w25q32.Lock = 0;
}

