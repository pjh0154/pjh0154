#include "cantus.h"
#include "ep.h"
#include "drv.h"
	
extern	 unsigned char	packet_read[256];

void Interrupt_disable(void)
{
	*R_INTEN = 0x0000000000; //all disable
    *R_INTMASKCLR = 0xffffffff;// all interrupt disable, except uart2
}

void Interrupt_enable(void)
{
    *R_INTEN = 0x0000008a;
    *R_INTMASKCLR = 0xffffff75;
}

void swreset(void)
{
	*R_PMCTRLEN |= (1<<7); 
	*R_PMCTRLEN |= (1<<7); 

	*R_RSTCTRL |= 1;
	*R_RSTCTRL |= 1;

	*R_PMCTRLEN |= (1<<7); 
	*R_PMCTRLEN |= (1<<7); 

	*R_RSTCTRL |= 1;
	*R_RSTCTRL |= 1;
}

BOOL fpga_loading(void)
{
	volatile unsigned int i=0;

	unsigned char *des;
	unsigned int flash_read_address;
	int file_size = 0;
	
	flash_read_address = FLASH_MEMORY_BLOCK0;
	des = (unsigned char *)FPGA_BASE_ADDR;
		
	nconfig_high();
	while(!nstatus());
	nconfig_low();
	while(nstatus());
	while(conf_done());
	nconfig_high();
	while(!nstatus());

	while(1)
	{
		 spi_read(READ_DATA, (unsigned int)flash_read_address,256);
		for(i = 0 ; i < 256 ; i++)
		{
			*des = packet_read[i];
			file_size++;
			if(conf_done()) 
			{
				debugprintf("CONF_DONE_OK\r\n");
				break;
			}
		}
		if(conf_done()) break;
		flash_read_address += 256;
		if(file_size > FPGA_FILE_SIZE+10)
		{
			debugprintf("FPGA_DOWNLOAD_FAIL\r\n");
			break;
		}
	}   
	*des = 0xaa;
	*des = 0xaa;
	debugprintf("file size = %d \r\n", file_size);
		
	
	if((file_size != FPGA_FILE_SIZE) && (!conf_done())) return FALSE;
	else return TRUE;
}

/*BOOL fpga_loading(void)
{
	FATFS fs;
	FIL* fp = NULL;
	FRESULT res;
	volatile unsigned int i=0;
	unsigned int br;
	unsigned char fdata;
	unsigned short *des;

	des = (unsigned short *)FPGA_BASE_ADDR;

	if(f_mount(DRIVE_NAND,&fs) != FR_OK)
	{
		debugprintf("Mount Error\r\n");
		while(1);
	}

	res = f_open(fp,FPGA_FILE_NAME,FA_OPEN_EXISTING | FA_READ);
	
	if(res != FR_OK)
	{
		debugprintf("file(%s) not found\r\n",FPGA_FILE_NAME);
		return FALSE;
	}
	nconfig_high();
	delayms(100);
	nconfig_low();
	delayms(1);
	while(nstatus());
	delayms(1);
    while(conf_done());
    delayms(1);
    nconfig_high();
	delayms(1);
    while(!nstatus());
    delayms(1);
	while(!conf_done()){
		f_read(fp,&fdata,1, &br);
		if(br != 1) break;
		*des = fdata;
		i++;
		if(i==(FPGA_FILE_SIZE+0x10)) break;
	}
	f_close(fp);
	debugprintf("file size = %d\r\n", i);

	f_mount(DRIVE_NAND,NULL);
	*R_NFMCFG |=(1<<16);
	
	if(i >= FPGA_FILE_SIZE+0x10) return FALSE;
	else return TRUE;
}
*/
void nconfig_low(void)
{
	*R_P3oLOW = (1<<2);
}
void nconfig_high(void)
{
    *R_P3oHIGH = (1<<2);
}

unsigned int nstatus(void)
{
	unsigned int data = 0;
	data = (*R_P3iLEV >> 1) & 0x01;
    return data;
}

unsigned int conf_done(void)
{
    unsigned int data = 0;
	data = (*R_P3iLEV >> 3) & 0x01;
    return data;
}

void fpga_reset(void)
{
	*R_P3oLOW = (1<<0);
	delayms(500);
    *R_P3oHIGH = (1<<0);
}

void red_led(char *str)
{
	if(!strncmp(str, "ON ", 2)) *R_P4oLOW  = 0x80;
	else if(!strncmp(str, "OFF ", 3)) *R_P4oHIGH  = 0x80;
}
