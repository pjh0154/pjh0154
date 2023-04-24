#include "cantus.h"

#define SDC_PWR_BIT 0x04
#define CS2CTRL	((volatile unsigned int*)0x80000408)		//FPGA
#define CS0CTRL	((volatile unsigned int*)0x80000400)		//FPGA
#define CS1CTRL	((volatile unsigned int*)0x8000040C)		//FPGA
void evmboardinit();

void evmboardinit()
{
	//*(volatile U16*)0x80000404 = 0x0200;//BANK 1 8Bit
	*(volatile U16*)0x80000404 = 0x0201;//BANK 1 8Bit
	*R_PAF0 = 0xAAAA; // ADDRESS_DATA
	*R_PAF1 = 0xAAAA; // GPIO
	*R_PAF2 = 0XFAAE; // 2.0 : ALE0 / 2.1 : NC / 2.2 : SRAM_nRE / 2.3 : SRAM_nWE / 2.4 : SRAM_nCS0 / 2.5 : SRAM_nCS1 / 2.6 : NC / 2.7 : SPI_nCS
	*R_PAF3 = 0xFFFF; //  3.0 : FRESET / 3.1 : nSTATUS / 3.2 : nCONFIG / 3.4 : CONF_DONE
	*R_PAF4 = 0x0000; // uart0, uart1,uart2, uart3
	*R_PAF5 = 0xFFFF; // SPI  //5.0 : SPI_nRESET / 5.1 : SPI_nWP / 5.2 : SPI_SCLK / 5.3 : SPI_MOSI / 5.4 : SPI_MISO
	*R_PAF6 = 0x0300; // PIO   

	*R_P0oDIR = 0xFF;	
	*R_P1oDIR = 0xFF;
	*R_P2oDIR = 0xFF;
	//*R_P2iDIR = 0x22;
	*R_P3oDIR = 0xF5;
	*R_P3iDIR = 0x0A;
	*R_P4oDIR = 0x55;  // UART 1,3,5,7 TX, 2,4,6,8 RX
	*R_P5oDIR = 0xEF; 
	*R_P5iDIR = 0x10;
	*R_P6oDIR = 0x10;

	//*CS2CTRL = 0x0301;	// fpga data bus : 16bit
	
	*CS2CTRL = 0xfff1;	// fpga data bus : 16bit
	*CS0CTRL = 0xfff0;
	*CS1CTRL = 0xfff1;
}
