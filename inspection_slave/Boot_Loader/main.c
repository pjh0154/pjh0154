#include "../include/cantus.h"
#include "ep.h"

extern unsigned char  Ymodem_Receive(unsigned int *p_size);
void download_main();
void evmboardinit();
void mass_storage_main();
void update_app(void);
void firmware_loading(void);

char Key_Check(void);
void main_Menu(void);
void SerialDownload(void);

#define NORFLASH_SECTOR0	(volatile unsigned char*)0x00000000
#define NORFLASH_SECTOR1	(volatile unsigned char*)0x00010000
#define NORFLASH_SECTOR2	(volatile unsigned char*)0x00020000
#define NORFLASH_SECTOR3	(volatile unsigned char*)0x00030000
#define NORFLASH_SECTOR4	(volatile unsigned char*)0x00040000
#define NORFLASH_SECTOR5	(volatile unsigned char*)0x00050000
#define NORFLASH_SECTOR6	(volatile unsigned char*)0x00060000
#define NORFLASH_SECTOR7	(volatile unsigned char*)0x00070000

char aFileName[FILE_NAME_LENGTH];
unsigned char file_size[16];
char key_value = 0;
char fw_dn_flag = 0;
long file_size_count = 0;
int size_flag = 0;
char start_flag = 0;
int sector_size = 0;
char fpga_dw_flag = 0;

int main()
{
	int cnt;

	evmboardinit();
	InitInterrupt();
	UartConfig(0,115200,DATABITS_8,STOPBITS_1,UART_PARNONE);
	UartConfig(1,115200,DATABITS_8,STOPBITS_1,UART_PARNONE);
	setdebugchannel(0);
	debugstring("================================================\r\n");
	debugprintf(" ES260 BootLoader R1.1 System Clock(%dMhz)\r\n",GetAHBclock()/1000000);
	debugstring("================================================\r\n");
	
	U8 norid[2];

	norflash_readid(norid);

	int nor_size = norflash_get_size();	
	
	debugprintf("Nor-Flash Size : 0x%x\r\n",nor_size);	
	
	sector_size = norflash_get_sector_size();
	debugprintf("Sector Size : %d\r\n",sector_size);		
	start_flag = 0;	
	fpga_dw_flag = 0;
	key_value = Key_Check();
	
	if(key_value)
	{
		main_Menu();
	}
	
	void (*AppFunc)();
	
	dcache_invalidate_way();
	cnt = 0;

/* 	U8 norid[2];


	norflash_readid(norid);

	int nor_size = norflash_get_size();
	
	debugprintf("Nor-Flash Size : 0x%x\r\n",nor_size); */


	U32 app_loaded_offset;
	
	if(nor_size == 0x20000) 
	{	
													// 128Kbyte
		app_loaded_offset = 32*1024;				// 2nd sector (0x8000)
	}
	else 
	{
		if(sector_size == 0x8000)		// Cantus512A sector 32Kbyte
		{									
			app_loaded_offset = 32*1024;
		}
		else										// Cantus512 sector 64Kbyte
		{
			app_loaded_offset = 64*1024;			
		}				// 2nd sector (0x10000)
	}
	
	AppFunc = (void (*)())(*(U32*)(app_loaded_offset));// 2nd sector


	if( ((U32)AppFunc <app_loaded_offset) || ((U32)AppFunc > nor_size))	{

		debugprintf("AppFunc(0x%x) is invalid\r\n",AppFunc);
	}
	else{
		debugstring("Run Application\r\n");
		AppFunc();
	}

	return 0;
}


void __attribute__ ((unused,section (".flash_code"))) update_app()
{
	U8 r_buf[1024];
	int ret, i, k;
	
	for(i=0;i<2;i++) debugprintf("nand 7 data[%d] = %02x \r\n ",i,*(NORFLASH_SECTOR7 +i));
	
	

	if(*(NORFLASH_SECTOR7) == 0xaa) {

		ret=norflash_erase((U32)NORFLASH_SECTOR1,64*1024*3);
		
		if(ret != 0) debugprintf("Erase Fail Ret = %d \r\n",ret);
		else{}
		debugprintf("nand update\r\n");
		// copy from sector 4,5,6 to sector 1,2,3
		
		for(k=0; k<64*3; k++) {
			for(i=0; i<1024; i++) {
				r_buf[i] = *(NORFLASH_SECTOR4+(k*1024)+i);
			}
			ret = norflash_write(r_buf,(U32)NORFLASH_SECTOR1 + k*1024,1024);
			if(ret != 0)
			{
				debugprintf("Write Fail Ret = %d \r\n",ret);
			}
		}
		r_buf[0] = 0x33;
		r_buf[1] = 0x77;
		ret = norflash_erase((U32)NORFLASH_SECTOR7,64*1024);
		ret = norflash_write(r_buf, (U32)NORFLASH_SECTOR7, 2);
		debugprintf("Firmware Update OK !!\r\n");
	}
	else if(*(NORFLASH_SECTOR7) == 0x55) {
	
		r_buf[0] = 0x33;
		r_buf[1] = 0x77;
		ret = norflash_erase((U32)NORFLASH_SECTOR7,64*1024);
		ret = norflash_write(r_buf, (U32)NORFLASH_SECTOR7, 2);
	}
	else {}
	
	
}

char Key_Check(void)
{
	char value=0;
	int count1=0;
	int flag=0;
	char sub_tx='.';

	debugprintf("\r\nPress 'D' key for F/W Download");	
	
	while(1)
	{
			if(UartGetCh(DEBUG_UART_PORT, &value) != 0 ) 
			{			
				if((value == 'D') || (value == 'd'))
				{
				value = 1;
				fw_dn_flag = 0;
				break;
				}
			}
			if(UartGetCh(UART_SUB_HANDLE, &value) != 0 ) 
			{
				if((value == 'D') || (value == 'd'))
				{
				value = 1;
				fw_dn_flag = 1;
				break;
				}
			}		
		count1++;
		if(count1 >= 750000)
		{
			count1 = 0;
			flag++;
			UartPutCh(DEBUG_UART_PORT, sub_tx);
			UartPutCh(UART_SUB_HANDLE, sub_tx);
		}

		if(flag >= 5){
			value = 0;
			break;
		}			
	}
return value;	
}

void main_Menu(void)
{
	char sub_tx = 'T';
	char key = 0;
	//unsigned int count = 1000000;
	unsigned int count1 = 100000;

	while(1)
	{
		if(!fw_dn_flag)
		{
			debugprintf("\r\n=================== Main Menu ============================\r\n\n");
			debugprintf("  Download image to the internal Flash ----------------- 1\r\n\n");
			debugprintf("  Download image to the external Flash ----------------- 2\r\n\n");			
			debugprintf("  Execute the loaded application ----------------------- 3\r\n\n");
			debugprintf("==========================================================\r\n\n");
		}	
		while(count1--)
		{
			if(UartPutCh(UART_SUB_HANDLE, sub_tx)!=0) break;
			asm("nop");
		}

		while (1)
		{
			key = 0;
			if(!fw_dn_flag)
			{
				if(UartGetCh(DEBUG_UART_PORT, &key)!=0) break;
			}
			else
			{	
				if(UartGetCh(UART_SUB_HANDLE, &key)!=0) break;	
			}
		}
		switch (key)
		{
			case '1' :
				if(!fw_dn_flag) Uart_rx_flush(DEBUG_UART_PORT);
				else Uart_rx_flush(UART_SUB_HANDLE);
				SerialDownload();
				break;
			case '2' :
				fpga_dw_flag	=	1;
				delayms(1);
				if(!fw_dn_flag) Uart_rx_flush(DEBUG_UART_PORT);
				else Uart_rx_flush(UART_SUB_HANDLE);		
				SerialDownload();
				break;
			case '3' :
				debugprintf("Start program execution......\r\n\n");		
				break;					
			default:
				debugprintf("Invalid Number ! ==> The number should be either 1, 2\r\n");
				break;				
		}	
		//if(key == '2') break;		
		if(key == '3') break;
		//if(start_flag) break;		
	}
}

void SerialDownload(void)
{
	int size = 0;
	char result;

	debugprintf("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	//debugprintf("Waiting for the file to be sent ... \n\r");
	
	result = Ymodem_Receive(&size);
	if(result == COM_OK)
	{
		debugprintf("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
		debugprintf(aFileName);
		debugprintf("\n\r Size: ");
		debugprintf(file_size);
		debugprintf(" Bytes\r\n");;
		debugprintf("--------------------------------\r\n");
		debugprintf("Start program execution......\r\n");	
		start_flag = 1;
	}
	else if (result == COM_LIMIT)
	{
		debugprintf("\n\n\rThe image size is higher than the allowed space memory!\n\r");
	}
	else if (result == COM_DATA)
	{
		debugprintf("\n\n\rVerification failed!\n\r");
	}
	else if (result == COM_ABORT)
	{
		debugprintf("\r\n\nAborted by user.\n\r");
	}
	else
	{
		debugprintf("\n\rFailed to receive the file!\n\r");
	}
}
