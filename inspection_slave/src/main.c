#include "cantus.h"
#include "timer.h"
#include "command.h"
#include "ep.h"
#include "drv.h"
#include "model.h"
#include "pg.h"
#include "adc.h"
#include "dac.h"

extern void evmboardinit();
void fpga_dummy_read(void);

int main()
{
	evmboardinit();
	InitInterrupt();
	
	dcache_invalidate_way();
	
	UartConfig(DEBUG_UART,115200,DATABITS_8,STOPBITS_1,UART_PARNONE);
	UartConfig(COM_UART,115200,DATABITS_8,STOPBITS_1,UART_PARNONE);
	setdebugchannel(DEBUG_UART);
	
	debugstring("================================================\r\n");
	debugprintf(" EP269_R10 %s %d,(System Clock %dMHz)\r\n", FIRMWARE_DATA, FIRMWARE_VERSION, GetAHBclock()/1000000);
	debugstring("================================================\r\n");
	//Interrupt_enable();
	*R_IINTMOD &= ~(1<<INTNUM_UART1);
	setinterrupt(INTNUM_UART1,uart1_isr);
	EnableInterrupt(INTNUM_UART1,TRUE);
	*(U32*)(((U32)R_UART1_BASE)+UART_IER) |= UIER_RDAIEN;
	
	setinterrupt(INTNUM_TIMER0,timer0isr);
	settimer(0,1);
	
	delayms(1);
	debugprintf("FPGA Loading...start\r\n");
 	if(fpga_loading()) debugprintf("FPGA Loading...OK\r\n");
	else 					debugprintf("FPGA Loading...Fail\r\n");
	fpga_dummy_read();
	fpga_reset(); 
	
	system_init();
	
	debugstring(PROMPT);
	while(1) 
	{
		uart_task();
		timer_10msec_enable_task();
	}
	return 0;
}

void fpga_dummy_read(void)
{
	unsigned short temp;
	int i;
	unsigned short *des = (unsigned short *)FPGA_BASE_ADDR_1;
	for(i=0; i<10; i++) temp = *des;
}

void system_init(void)
{	
	PG = (PG_HANDLE *)(PG_BASE);
	DAC = (DAC_HANDLE *)(DAC_BASE);
	ADC = (ADC_HANDLE *)(ADC_BASE);
	
	red_led("OFF");
	memset(&ensis_operation, 0, sizeof(ensis_operation));
	
	output_delay = OUTPUT_DELAY_DEFAULT;
	pg_handle->output_delay_l = (unsigned short)((output_delay >> 0) & 0x0000FFFF);
	pg_handle->output_delay_h = (unsigned short)((output_delay >> 16) & 0x0000FFFF); 
	
	if(DAC_INIT())		debugprintf("DAC_INIT_OK\r\n");
	else						debugprintf("DAC_INIT_Fail\r\n");
	DAC_0V_SETTING();	

	/*if(CALIBRATION_LOAD())
	{
		debugprintf("Calibration LOAD OK\r\n");
		CALIBRATION_PRINT();
	}
	else debugprintf("Calibration LOAD Fail\r\n");
		
	memset(&adc_sensing_value, 0, sizeof(adc_sensing_value));
	if(ADC_INIT())
	{
		debugprintf("ADC Init OK\r\n");
		debugprintf("ADC REG VALUE PRINT\r\n");
		ADC_REG_PRINT();
	}
	else debugprintf("ADC Init Fail\r\n");*/
}
