#include "adc.h"

static BOOL ADC_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS address, unsigned char *init_value);
static unsigned char ADC_CRC8(unsigned char *data, int size);
	
BOOL ADC_REG_READ(ADC_REG_ADDRESS *address, unsigned short *read_data)
{
	static unsigned short control_backup;
	int i;
	
	control_backup = adc_handle->control;
	adc_handle->control = 0;

	i = 0;
	while(1)
	{
		if(adc_handle->status & ADC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}
	
	adc_handle->reg_address = (unsigned short)*address;
	adc_handle->control = ADC_REG_READ_EN;

	i = 0;
	while(1)
	{
		if(adc_handle->status & ADC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}

	*read_data = adc_handle->reg_read_data;	
	adc_handle->control = control_backup;
	
	return TRUE;
}

BOOL ADC_REG_WRITE(ADC_REG_ADDRESS *address, unsigned short *write_data)
{
	static unsigned short control_backup;
	int i;
	
	control_backup = adc_handle->control;
	adc_handle->control = 0;

	i = 0;
	while(1)
	{
		if(adc_handle->status & ADC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}
	
	adc_handle->reg_address = (unsigned short)*address;
	adc_handle->reg_write_data = *write_data;
	adc_handle->control = ADC_REG_WRITE_EN;

	i = 0;
	while(1)
	{
		if(adc_handle->status & ADC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}

	adc_handle->control = control_backup;
	
	return TRUE;
}

void ADC_REG_PRINT(void)
{
	ADC_REG_ADDRESS address;
	static unsigned short data;
		
	address = ID;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = STATUS;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = INPMUX;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = PGA;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = DATARATE;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = REF;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = IDACMAG;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = IDACMUX;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = VBIAS;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = SYS;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = OFCAL0;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = OFCAL1;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = OFCAL2;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = FSCAL0;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = FSCAL1;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = FSCAL2;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = GPIODAT;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
	address = GPIOCON;
	if(ADC_REG_READ(&address, &data)) debugprintf("Addr = 0x%x, Reg Read data = 0x%x\r\n", address, (unsigned int)data);
	else debugprintf("Addr = 0x%x Read Fail\r\n", address);
}

BOOL ADC_INIT(void)
{
	static unsigned char write_data;
	int i,j;
	
	adc_handle->control = ADC_RESET;
	
	i = 0;
	while(1)
	{
		if(adc_handle->status & ADC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}
	
	for(j=0; j<0x12; j++)
	{
		if(ADC_REG_SETTING_VALUE_LOAD((ADC_REG_ADDRESS)j, &write_data))
		{
			adc_handle->reg_address = (unsigned short)j;		
			adc_handle->reg_write_data = (unsigned short)write_data;
			adc_handle->control = ADC_REG_WRITE_EN;
			i = 0;
			while(1)
			{
				if(adc_handle->status & ADC_READY) break;
				delayms(1);
				i++;
				if(i > 5) return FALSE;				
			}
			debugprintf("Addr = 0x%x, Reg Write Data = 0x%x\r\n", j, write_data);
		}
	}

	adc_handle->auto_conversion_delay_l = (unsigned short)((ADC_AUTO_5MHz_CLOCK_DELAY >> 0) & 0x0000FFFF);
	adc_handle->auto_conversion_delay_h = (unsigned short)((ADC_AUTO_5MHz_CLOCK_DELAY >> 16) & 0x0000FFFF);
	
	i = 0;
	while(1)
	{
		if(adc_handle->status & ADC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}
	
	adc_handle->control = ADC_AUTO_READ_EN;
	
	return TRUE;
}

static BOOL ADC_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS address, unsigned char *init_value)
{
	switch(address)
	{
		case ID:
			return FALSE;
		case STATUS:
			*init_value = 0x00;
			return TRUE;
		case DATARATE:
			//*init_value = 0x3B;
			*init_value = 0x3D;
			return TRUE;
		case REF:
			*init_value = 0x09;
			return TRUE;
		case SYS:
			*init_value = 0x12;
			return TRUE;
		case INPMUX:
		case PGA:
		case IDACMAG:
		case IDACMUX:
		case VBIAS:
		case OFCAL0:
		case OFCAL1:
		case OFCAL2:
		case FSCAL0:
		case FSCAL1:
		case FSCAL2:
		case GPIODAT:
		case GPIOCON:
		default:
			return FALSE;
	}
}

#define ADC_UNIT ((float)0.000000298023224)
#define ADC_RATIO_SIG ((float)6.6)
#define ADC_RATIO_EL ((float)11.0)

void ADC_AUTO_DATA_READ(void)
{
	unsigned short channel;
	unsigned int data;
	unsigned char crc;
	float ratio;
	
	for(channel = 0; channel < 56; channel++)
	{
		adc_handle->auto_mux_select = channel;
		data = (adc_handle->auto_conversion_data_h << 16) | (adc_handle->auto_conversion_data_l);
		crc = ADC_CRC8((unsigned char *)&data, sizeof(data));
		if(!crc)
		{
			if((channel == 0) || (channel == 5) || (channel == 7) || (channel == 12) || (channel == 19)
			|| (channel == 26) || (channel == 32) || (channel == 39) || (channel == 46) || (channel == 53))
			{
				ratio = ADC_RATIO_EL;
			}
			else ratio = ADC_RATIO_SIG;
			
			if(data & 0x80000000)
			{
				adc_sensing_value.value[channel] = ((((float)((((~data) >> 8) & 0x007FFFFF)+1)) * (-1.0)) * ADC_UNIT * ratio * 1000.0);
			}
			else adc_sensing_value.value[channel] = ((((float)((data >> 8) & 0x007FFFFF)) * (1.0)) * ADC_UNIT * ratio * 1000);
		}
		else debugprintf("ADC CRC Error\r\n");
	}
	
	adc_handle->control = (ADC_AUTO_READ_EN | ADC_AUTO_READ_DONE_CLEAR);
}

static unsigned char ADC_CRC8(unsigned char *data, int size)
{
	unsigned char crc = 0x00;
	unsigned char *data_ptr = data + (size - 1);
	int i,j;
	
	for(j=size; j; j--)
	{
		crc ^= (*data_ptr << 8);
		for(i=8; i; i--)
		{
			if(crc & 0x8000) crc ^= (0x1070 << 3);
			crc <<= 1;
		}
		data_ptr--;
	}
	
	return crc;
}
