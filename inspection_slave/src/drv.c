#include "cantus.h"
#include "ep.h"
#include "drv.h"
	
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
	while(!nstatus());
	nconfig_low();
	while(nstatus());
	while(conf_done());
	nconfig_high();
	while(!nstatus());
	while(1){
		f_read(fp,&fdata,1, &br);
		if(br != 1) break;
		*des = fdata;
		i++;
	}
	f_close(fp);
	debugprintf("file size = %d\r\n", i);
		
	f_mount(DRIVE_NAND,NULL);
	*R_NFMCFG |=(1<<16);
	
	if((i != FPGA_FILE_SIZE) && (conf_done())) return FALSE;
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
	*R_P2oLOW = (1<<4);
}
void nconfig_high(void)
{
    *R_P2oHIGH = (1<<4);
}

unsigned int nstatus(void)
{
	unsigned int data = 0;
	data = (*R_P2iLEV >> 5) & 0x01;
    return data;
}

unsigned int conf_done(void)
{
    unsigned int data = 0;
	data = (*R_P2iLEV >> 1) & 0x01;
    return data;
}

void fpga_reset(void)
{
	*R_P2oLOW = (1<<7);
	delayms(500);
    *R_P2oHIGH = (1<<7);
}

void red_led(char *str)
{
	if(!strncmp(str, "ON ", 2)) *R_P4oLOW  = 0x80;
	else if(!strncmp(str, "OFF ", 3)) *R_P4oHIGH  = 0x80;
}
