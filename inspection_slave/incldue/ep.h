#pragma once

#define PROMPT						"[EP26X]"
#define FIRMWARE_DATA 		"2021.12.02 "
#define FIRMWARE_VERSION 	"1.0"

#define FPGA_BASE_ADDR (volatile unsigned short *)0x60080000

#define NORFLASH_SECTOR0	(volatile unsigned char*)0x00000000
#define NORFLASH_SECTOR1	(volatile unsigned char*)0x00010000
#define NORFLASH_SECTOR2	(volatile unsigned char*)0x00020000
#define NORFLASH_SECTOR3	(volatile unsigned char*)0x00030000
#define NORFLASH_SECTOR4	(volatile unsigned char*)0x00040000
#define NORFLASH_SECTOR5	(volatile unsigned char*)0x00050000
#define NORFLASH_SECTOR6	(volatile unsigned char*)0x00060000
#define NORFLASH_SECTOR7	(volatile unsigned char*)0x00070000

#define FPGA_FILE_SIZE 1191788
#define FPGA_FILE_NAME "0:/fpga/EP261_EP262.rbf"

#define MAX_PATTERN_SIZE 1

#pragma pack(1)
typedef struct{
	unsigned short 	inversion;				
	short 		  	volt_high;				
	short 		 	volt_low;					
	unsigned int	time_sector[5];
}ENSIS_SIGNAL_CONFIG;	

typedef struct{
	char 		 	pat_name[32];		
	ENSIS_SIGNAL_CONFIG	sig_conf[36];		//Chnanel : 36
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
