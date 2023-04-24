

#include "cantus.h"

#define SDC_PWR_BIT 0x04
#define CS2CTRL	((volatile unsigned int*)0x80000408)		//FPGA

void evmboardinit()
{
	//*(volatile U16*)0x80000404 = 0x0200;//BANK 1 8Bit
	*R_PAF0 = 0xAAAA; // ADDRESS_DATA
	*R_PAF1 = 0xAAAA; // GPIO
	*R_PAF2 = 0xEFAE; // 
	*R_PAF3 = 0xAAAF; // 
	*R_PAF4 = 0xF000; // uart0, uart1,uart2, uart3
	*R_PAF5 = 0xFFFF;
	*R_PAF6 = 0x0300; // PIO   

	*R_P0oDIR = 0xFF;	
	*R_P1oDIR = 0xFF;
	*R_P2oDIR = 0xDD;
	*R_P2iDIR = 0x22;
	*R_P3oDIR = 0xFF;
	*R_P4oDIR = 0xD5;  // UART 1,3,5,7 TX, 2,4,6,8 RX
	*R_P5oDIR = 0xFF;
	*R_P6oDIR = 0x10;

	//*CS2CTRL = 0x0301;	// fpga data bus : 16bit
	
	*CS2CTRL = 0xfff1;	// fpga data bus : 16bit
}
