#include "cantus.h"
#include "timer.h"
#include "adc.h"
#include "dac.h"

unsigned char timer_10msec_enable;
unsigned int count;
unsigned char LED_OP_FLAG = 1;

void timer0isr(void)
{
	timer_10msec_enable = ENABLE;
}

void timer_10msec_enable_task(void)
{	
	if(timer_10msec_enable)
	{
		timer_10msec_enable = DISABLE;
		if(count >= 500)
		{
			if(LED_OP_FLAG)
			{
				if(*R_P4oLEV & 0x40) *R_P4oLOW  = 0x40;
				else *R_P4oHIGH  = 0x40;
			}
			else *R_P4oHIGH  = 0xC0;
			count = 0;
		}
		else count++;
		
		if(adc_handle->status & ADC_AUTO_READ_DONE) ADC_AUTO_DATA_READ();
	}
}
