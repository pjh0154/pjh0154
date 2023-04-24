#pragma once

#define PROMPT						"[EP269]"
#define FIRMWARE_DATA 		"2022.04.15 "
#define FIRMWARE_VERSION 	10


//#define FPGA_BASE_ADDR (volatile unsigned short *)0x60080000
#define FPGA_BASE_ADDR (volatile unsigned short *)0x40000000
#define FPGA_BASE_ADDR_1 (volatile unsigned short *)0x60000000

#define NORFLASH_SECTOR0	(volatile unsigned char*)0x00000000
#define NORFLASH_SECTOR1	(volatile unsigned char*)0x00010000
#define NORFLASH_SECTOR2	(volatile unsigned char*)0x00020000
#define NORFLASH_SECTOR3	(volatile unsigned char*)0x00030000
#define NORFLASH_SECTOR4	(volatile unsigned char*)0x00040000
#define NORFLASH_SECTOR5	(volatile unsigned char*)0x00050000
#define NORFLASH_SECTOR6	(volatile unsigned char*)0x00060000
#define NORFLASH_SECTOR7	(volatile unsigned char*)0x00070000

#define FPGA_FILE_SIZE 1191788

#define MAX_PATTERN_SIZE 1

#pragma pack(1)
typedef struct{
	unsigned int 	inversion;	
	short 		  	volt_high;				
	short 		 	volt_low;					
	unsigned int	time_sector[5];			//	0 : STEP1 // 1: STEP2 // 2 : STEP3 // 3: STEP4 // 4: PERIOD
}ENSIS_SIGNAL_CONFIG;	

typedef struct{
	char 		 						pat_name[64];		
	unsigned int 					led_on_off;
	ENSIS_SIGNAL_CONFIG	sig_conf[46];		//sig_conf[0] : ELVDD, sig_conf[1] : ELVSS // sig_conf[2]~[45]Chnanel : 44
}ENSIS_PATTERN_CONFIG;	

typedef struct{
	unsigned char 			pat_count;		
	ENSIS_PATTERN_CONFIG	pat_conf[1];
}ENSIS_MODEL_GROUP;	

ENSIS_MODEL_GROUP ensis_model_gp; 

typedef struct{
	unsigned short pattern_id;
}ENSIS_OPERATION;

ENSIS_OPERATION ensis_operation;
#pragma pack(4)

/* #define SECTOR_1	((volatile unsigned char*)0x00010000)
#define SECTOR_2	((volatile unsigned char*)0x00020000)
#define SECTOR_3	((volatile unsigned char*)0x00030000)
#define SECTOR_4	((volatile unsigned char*)0x00040000)
#define SECTOR_5	((volatile unsigned char*)0x00050000)
#define SECTOR_6	((volatile unsigned char*)0x00060000)
#define SECTOR_7	((volatile unsigned char*)0x00070000) */

#define DEBUG_UART_PORT UART0
#define UART_SUB_HANDLE UART1


