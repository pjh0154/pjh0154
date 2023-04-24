#include "cantus.h"
#include "timer.h"
#include "adc.h"
#include "dac.h"

unsigned char timer_10msec_enable;
unsigned int count;

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
			
			if(*R_P6oLEV & 0x10) *R_P6oLOW  = 0x10;
			else *R_P6oHIGH  = 0x10;
			
			count = 0;
		}
		else count++;
		
		//if(adc_handle->status & ADC_AUTO_READ_DONE) ADC_AUTO_DATA_READ();
	}
}
