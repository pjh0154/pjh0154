#include "../include/adc.h"
#include "../include/fpga_reg.h"
#include "../include/ep.h"

extern int cprf;

//static int ADC_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS_NUM address, unsigned char *init_value);
static int ADC_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS_NUM address, unsigned char *init_value)
{
	switch(address)
	{
		case ID:
			return 0;
		case STATUS:
			*init_value = 0x00;
			return 1;
		case PGA:
			*init_value = 0x00;
			return 1;			
		case DATARATE:
			//*init_value = 0x3D;
			//*init_value = 0x39; //about 160ms
			//*init_value = 0x29; //about 440ms
			*init_value = 0x38; //about 440ms			
			return 1;
		case REF:
			*init_value = 0x09;
			//*init_value = 0x10;
			return 1;
		case SYS:
			*init_value = 0x12;
			return 1;
		case VBIAS:
		case OFCAL0:
		case OFCAL1:
		case OFCAL2:
		case INPMUX:
		//case PGA:
		case IDACMAG:
		case IDACMUX:
		case FSCAL0:
		case FSCAL1:
		case FSCAL2:
		case GPIODAT:
		case GPIOCON:
		default:
			return 0;
	}
}

static int ADC_RM_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS_NUM address, unsigned char *init_value)
{
	switch(address)
	{
		case ID:
			return 0;
		case STATUS:
			*init_value = 0x00;
			return 1;
		case PGA:
			//*init_value = 0x01;
			*init_value = 0x00;
			//PGA_GAIN x2
			return 1;			
		case DATARATE:
			//*init_value = 0x3D;
			//*init_value = 0x39; //about 160ms
			//*init_value = 0x29; //about 440ms
			*init_value = 0x25; //about 440ms			
			return 1;
		case REF:
			*init_value = 0x10;
			//*init_value = 0x1A;
			return 1;
		case SYS:
			*init_value = 0x12;
			return 1;
		case VBIAS:
		case OFCAL0:
		case OFCAL1:
		case OFCAL2:
		case INPMUX:
		//case PGA:
		case IDACMAG:
		case IDACMUX:
		case FSCAL0:
		case FSCAL1:
		case FSCAL2:
		case GPIODAT:
		case GPIOCON:
		default:
			return 0;
	}
}

void ads124s08_sensing_init(void)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data;    
    ads124s08_sensing->ADC_CONTROL = ADC_Control_RESET;
	while(1)
	{
		if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_RESET))
		{
			ready = 1;
			break;
		}				
		usleep(5000);
		j++;
		//if(j > 5) return 0;
		if(j > 1000) break;
	}
	usleep(50000);
	if(!ready)	
	{
		printf("ADS124_RESET_FAIL\r\n");
		return;
	}    
    printf("----------ads124s08_sensing_init----------\r\n");			
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_sensing->ADC_REG_ADDRESS = i;
        ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;
		while(1)
		{
			if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(5000);
			j++;
			//if(j > 5) return 0;
			if(j > 1000) break;
		}	
		if(!ready)	
		{
			printf("ADS124_READ_FAIL\r\n");
			return;
		} 
        printf("REG%d = %x\r\n", i, ads124s08_sensing->ADC_REG_READ);
    } 
    printf("------------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
        if(ADC_REG_SETTING_VALUE_LOAD((ADC_REG_ADDRESS_NUM)i, &write_data))
		{	
            printf("i = %d / write_data = %x\r\n", i, write_data);
            ads124s08_sensing->ADC_REG_ADDRESS = i;
            ads124s08_sensing->ADC_REG_WRITE = write_data;
            ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_WRITE; 
			j=0;
			ready = 0;			
			while(1)
			{
				if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_WRITE))
				{
					ready = 1;
					break;
				}				
				usleep(5000);
				j++;
				//if(j > 5) return 0;
				if(j > 1000) break;
			}
			if(!ready)	
			{
				printf("ADS124_WRITE_FAIL\r\n");
				return;
			} 					          
        } 
    }
    printf("------------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_sensing->ADC_REG_ADDRESS = i;
        ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;			
		while(1)
		{
			if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(5000);
			j++;
			//if(j > 5) return 0;
			if(j > 1000) break;
		}
		if(!ready)	
		{
			printf("ADS124_READ_FAIL\r\n");
			return;
		} 		
        //data = ads124s08_sensing->ADC_REG_READ;
        printf("REG%d = %x\r\n", i, ads124s08_sensing->ADC_REG_READ);
    }
    //ads124s08_sensing->ADC_CONTROL = ADC_Control_START_PIN_HIGH;     
}

void ads124s08_ads124s08_rm_init(void)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data;    
    ads124s08_rm->ADC_CONTROL = ADC_Control_RESET;

	while(1)
	{
		if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_RESET))
		{
			ready = 1;
			break;
		}				
		usleep(5000);
		j++;
		//if(j > 5) return 0;
		if(j > 1000) break;
	}
	usleep(50000);
	if(!ready)	
	{
		printf("ADS124_RESET_FAIL\r\n");
		return;
	}

    printf("----------ads124s08_rm_init----------\r\n");	  
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_rm->ADC_REG_ADDRESS = i;
        //ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
        ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;
		while(1)
		{
			if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(5000);
			j++;
			//if(j > 5) return 0;
			if(j > 1000) break;
		}
		if(!ready)	
		{
			printf("ADS124_RM_READ_FAIL\r\n");
			return;
		}				
        //data = ads124s08_sensing->ADC_REG_READ;
        printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
    } 
    printf("-------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
        if(ADC_RM_REG_SETTING_VALUE_LOAD((ADC_REG_ADDRESS_NUM)i, &write_data))
		{	
            printf("i = %d / write_data = %x\r\n", i, write_data);
            ads124s08_rm->ADC_REG_ADDRESS = i;
            ads124s08_rm->ADC_REG_WRITE = write_data;
            //ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
            ads124s08_rm->ADC_CONTROL = ADC_Control_REG_WRITE; 
			j=0;
			ready = 0;
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_WRITE))
				{
					ready = 1;
					break;
				}				
				usleep(5000);
				j++;
				//if(j > 5) return 0;
				if(j > 1000) break;
			}
			if(!ready)	
			{
				printf("ADS124_RM_WRITE_FAIL\r\n");
				return;
			}								          
        } 
    }
    printf("-------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_rm->ADC_REG_ADDRESS = i;
        //ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
        ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;
		while(1)
		{
			if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(5000);
			j++;
			//if(j > 5) return 0;
			if(j > 1000) break;
		}
		if(!ready)	
		{
			printf("ADS124_RM_READ_FAIL\r\n");
			return;
		}			
        //data = ads124s08_sensing->ADC_REG_READ;
        printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
    }
    //ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;       
}

void ads124s08_ads124s08_rm_test(char data)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data;    
    ads124s08_rm->ADC_CONTROL = ADC_Control_RESET;

	while(1)
	{
		if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_RESET))
		{
			ready = 1;
			break;
		}				
		usleep(5000);
		j++;
		//if(j > 5) return 0;
		if(j > 200) break;
	}

	if(!ready)	
	{
		printf("ADS124_RESET_FAIL\r\n");
		return;
	}

    printf("----------ads124s08_rm_init----------\r\n");	  
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_rm->ADC_REG_ADDRESS = i;
        ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
        ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;
		while(1)
		{
			if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(5000);
			j++;
			//if(j > 5) return 0;
			if(j > 200) break;
		}
		if(!ready)	
		{
			printf("ADS124_RM_READ_FAIL\r\n");
			return;
		}				
        //data = ads124s08_sensing->ADC_REG_READ;
        printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
    } 
    printf("-------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
		if(i == 4)
		{
			printf("i = %d / input_data = %x\r\n", i, data);
			ads124s08_rm->ADC_REG_ADDRESS = i;
			ads124s08_rm->ADC_REG_WRITE = data;
			ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
			ads124s08_rm->ADC_CONTROL = ADC_Control_REG_WRITE; 
			j=0;
			ready = 0;
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_WRITE))
				{
					ready = 1;
					break;
				}				
				usleep(5000);
				j++;
				//if(j > 5) return 0;
				if(j > 200) break;
			}
			if(!ready)	
			{
				printf("ADS124_RM_WRITE_FAIL\r\n");
				return;
			}								          
		}
		else
		{
			if(ADC_RM_REG_SETTING_VALUE_LOAD((ADC_REG_ADDRESS_NUM)i, &write_data))
			{	
				printf("i = %d / write_data = %x\r\n", i, write_data);
				ads124s08_rm->ADC_REG_ADDRESS = i;
				ads124s08_rm->ADC_REG_WRITE = write_data;
				ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
				ads124s08_rm->ADC_CONTROL = ADC_Control_REG_WRITE; 
				j=0;
				ready = 0;
				while(1)
				{
					if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_WRITE))
					{
						ready = 1;
						break;
					}				
					usleep(5000);
					j++;
					//if(j > 5) return 0;
					if(j > 200) break;
				}
				if(!ready)	
				{
					printf("ADS124_RM_WRITE_FAIL\r\n");
					return;
				}								          
			}
		} 
    }
    printf("-------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_rm->ADC_REG_ADDRESS = i;
        ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
        ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;
		while(1)
		{
			if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(5000);
			j++;
			//if(j > 5) return 0;
			if(j > 200) break;
		}
		if(!ready)	
		{
			printf("ADS124_RM_READ_FAIL\r\n");
			return;
		}			
        //data = ads124s08_sensing->ADC_REG_READ;
        printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
    }
    //ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;       
}

unsigned short ADC_CRC8(unsigned char *data, int size)
{
 	unsigned short crc = 0x0000;
	unsigned char *data_ptr = data + (size - 1);
	int i,j;
	for(j=size; j; j--)
	{
		crc ^= (*data_ptr << 8);
		for(i=8; i; i--)
		{
			if(crc & 0x8000)
			{
				crc ^= (0x1070 << 3); 
			}
			crc <<= 1;
		}
		data_ptr--;
	}
	
	return crc; 
}

void ADC_AUTO_DATA_READ(void)
{
	int i = 0;
	int j = 0;
	int ii = 0;
	int iii = 0;
	int ready = 0;
	int mux_sel = 7;
	unsigned int adc_read_data_0;
	float real_read_data_0;
	unsigned int adc_read_data_1;
	float real_read_data_1;
	float adc_data_0_a;	
	float adc_data_1_r;								
	unsigned short crc;	

	ads124s08_sensing->ADC_AUTO_DELAY = 2000;						

	if((ads124s08_sensing->ADC_STATUS & ADC_STATUS_AUTO_READ_DONE))	ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE | ADC_Control_AUTO_READ_CLEAR;	
	else ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE;

	while(1)
	{
		if((ads124s08_sensing->ADC_STATUS & ADC_STATUS_AUTO_READ_DONE))
		{
			ready = 1;
			break;
		}				
		usleep(1000);
		j++;
		//if(j > 5) return 0;
		if(j > 1000) break;
	}
	usleep(1000);
	if(ready)	
	{
		for(ii=39 ; ii >= 0 ; ii--)
		{
			ads124s08_sensing->ADC_AUTO_DATA_SEL = ii;	
			adc_read_data_0 = ads124s08_sensing->ADC_AUTO_DATA;						
			crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
			if(!crc)
			{
				/*if((ii == ADC_IVDD ) || (ii ==ADC_IVSS ))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)) * (float)0.25*(1+(820000/13000)*0.1)/3;
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)) * (float)0.25*(1+(820000/13000)*0.1)/3;							
				}*/
				if((ii == ADC_IVDD ))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * avdd_cur_cal.cur_1a_ratio+avdd_cur_cal.cur_1a_offset)+avdd_cur_cal.cur_1a_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * avdd_cur_cal.cur_1a_ratio+avdd_cur_cal.cur_1a_offset)+avdd_cur_cal.cur_1a_user_offset);	
				}
				else if((ii ==ADC_IVSS ))
				{
					//if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)) * (float)0.25*(1+(820000/13000)*0.1)/3;
					//else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)) * (float)0.25*(1+(820000/13000)*0.1)/3;						
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.cur_1a_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.cur_1a_user_offset);	
				}
				else if((ii ==ADC_IVDD_100mA))
				{
					//if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1))/(((((1000000/1000)+1)*0.1))/0.25);
					//else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1))/(((((1000000/1000)+1)*0.1))/0.25);
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * avdd_cur_cal.cur_100ma_ratio+avdd_cur_cal.cur_100ma_offset)+avdd_cur_cal.cur_100ma_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * avdd_cur_cal.cur_100ma_ratio+avdd_cur_cal.cur_100ma_offset)+avdd_cur_cal.cur_100ma_user_offset);											
				}
				else if((ii ==ADC_IVSS_100mA ))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_100ma_ratio+elvss_cur_cal.cur_100ma_offset)+elvss_cur_cal.cur_100ma_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_100ma_ratio+elvss_cur_cal.cur_100ma_offset)+elvss_cur_cal.cur_100ma_user_offset);							
				}
				else if((ii == ADC_I2C ) || (ii == ADC_LOGIC ))
				{
					//if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)*12)/ 5.3;
					//else real_read_data_0 = ((((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)*12)/5.3;
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)*2);
					else real_read_data_0 = ((((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)*2);													
				}
				else if((ii == ADC_VDD11) || (ii == ADC_VDD18) || (ii == ADC_DPSPARE1 ) || (ii == ADC_DPSAPRE2 ) || (ii == ADC_ELVDD ) || (ii == ADC_ELVSS ))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal4.adc_p_ratio+ads124_cal4.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal4.adc_p_ratio+ads124_cal4.adc_p_offset);							
				}
				else if((ii == ADC_AVDD)||(ii == ADC_ADD8_S)||(ii == ADC_ADD8_G)||(ii == ADC_VGH)||(ii == ADC_VGL)||(ii == ADC_VINT)||(ii == ADC_APSPARE1)||(ii == ADC_APSPARE2))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal3.adc_p_ratio+ads124_cal3.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal3.adc_p_ratio+ads124_cal3.adc_p_offset);							
				}
				else if((ii == ADC_LM_SPARE1)||(ii == ADC_LM_SPARE2))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal2.adc_p_ratio+ads124_cal2.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal2.adc_p_ratio+ads124_cal2.adc_p_offset);	
				}
				else if((ii == ADC_VOTP50)||(ii == ADC_PM_SPARE1)||(ii == ADC_MON1)||(ii == ADC_MON2)||(ii == ADC_MON3)||(ii == ADC_MON4)||(ii == ADC_MON5)||(ii == ADC_MON6))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal1.adc_p_ratio+ads124_cal1.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal1.adc_p_ratio+ads124_cal1.adc_p_offset);							
				}
				else if((ii == ADC_LDO_ELVDD)||(ii == ADC_LDO_OSC)||(ii == ADC_LDO_VGH)||(ii == ADC_LDO_VGL)||(ii == ADC_LDO_VINT)||(ii == ADC_VCIR)||(ii == ADC_VREF1)||(ii == ADC_VREG1))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal0.adc_p_ratio+ads124_cal0.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal0.adc_p_ratio+ads124_cal0.adc_p_offset);							
				}												
				else
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)*12;
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)*12;	
					//if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal3.adc_p_ratio+ads124_cal3.adc_p_offset);
					//else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal3.adc_p_ratio+ads124_cal3.adc_p_offset);						
				}				
				usleep(1000);
				//adc_sensing_value[i] = real_read_data_0;
				if((ii == ADC_IVDD ) || (ii ==ADC_IVSS ))	
				{
					adc_sensing_value[i] = real_read_data_0*10;
					cur_sensing_value[0] = real_read_data_0*100;

				}
				else if((ii ==ADC_IVDD_100mA ) || (ii ==ADC_IVSS_100mA ))	adc_sensing_value[i] = real_read_data_0*100;	

				else adc_sensing_value[i] = real_read_data_0*1000;
				i++;

			}
			else	printf("crc_rm_ng\r\n");
		}
														
	}
	else printf("reday_ng\r\n");	
}

void ADC_AUTO_DATA_READ_FOR_CAL(void)
{
	int i = 0;
	int j = 0;
	int ii = 0;
	int iii = 0;
	int ready = 0;
	int mux_sel = 7;
	unsigned int adc_read_data_0;
	int real_read_data_0;
	unsigned int adc_read_data_1;
	int real_read_data_1;
	int adc_data_0_a;	
	int adc_data_1_r;								
	unsigned short crc;	

	ads124s08_sensing->ADC_AUTO_DELAY = 10;						

	ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE | ADC_Control_AUTO_READ_CLEAR;
	while(1)
	{
		if((ads124s08_sensing->ADC_STATUS & (1<<2)))
		{
			ready = 1;
			break;
		}				
		usleep(5000);
		j++;
		//if(j > 5) return 0;
		if(j > 200) break;
	}

	if(ready)	
	{
		for(ii=39 ; ii >= 0 ; ii--)
		{
			ads124s08_sensing->ADC_AUTO_DATA_SEL = ii;	
			adc_read_data_0 = ads124s08_sensing->ADC_AUTO_DATA;						
			crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
			if(!crc)
			{
				if((ii == 17) || (ii ==7))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1) * (float)(-1.0);
					else real_read_data_0 = (((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0);							
				}
				else
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1) * (float)(-1.0);
					else real_read_data_0 = (((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0);	
				}				
				
				//adc_read_data_0 = (adc_read_data_0 >>8);
				//printf("ADC_OUT[%d / %d] = %x / %f\r\n", i, ii, adc_read_data_0, real_read_data_0);
				adc_sensing_value_for_cal[i] = real_read_data_0;
				i++;
			}
			else	printf("crc_rm_ng\r\n");
		}
														
	}
	else printf("reday_ng\r\n");		
}

void ADC_DATA_FOR_CAL_PRINT(void)
{
	printf("PM0 = %x\r\n", adc_sensing_value_for_cal[4]);
	printf("PM1 = %x\r\n", adc_sensing_value_for_cal[9]);
	printf("PM2 = %x\r\n", adc_sensing_value_for_cal[14]);
	printf("PM3 = %x\r\n", adc_sensing_value_for_cal[19]);
	printf("PM4 = %x\r\n", adc_sensing_value_for_cal[24]);
	printf("PM5 = %x\r\n", adc_sensing_value_for_cal[29]);
	printf("PM6 = %x\r\n", adc_sensing_value_for_cal[34]);
	printf("PM7 = %x\r\n", adc_sensing_value_for_cal[39]);
	printf("PM8 = %x\r\n", adc_sensing_value_for_cal[3]);
	printf("PM9 = %x\r\n", adc_sensing_value_for_cal[8]);
	printf("/////////////////////////////\r\n");	
	printf("LM0 = %x\r\n", adc_sensing_value_for_cal[13]);		
	printf("LM1 = %x\r\n", adc_sensing_value_for_cal[18]);
	printf("LM2 = %x\r\n", adc_sensing_value_for_cal[23]);
	printf("LM3 = %x\r\n", adc_sensing_value_for_cal[28]);
	printf("LM4 = %x\r\n", adc_sensing_value_for_cal[33]);
	printf("LM5 = %x\r\n", adc_sensing_value_for_cal[38]);
	printf("LM6 = %x\r\n", adc_sensing_value_for_cal[2]);
	printf("LM7 = %d\r\n", adc_sensing_value_for_cal[7]);		
	printf("/////////////////////////////\r\n");
	printf("IVDD = %x\r\n", adc_sensing_value_for_cal[22]);
	printf("IVSS = %x\r\n", adc_sensing_value_for_cal[32]);
	printf("/////////////////////////////\r\n");	
	printf("I2C_VO = %x\r\n", adc_sensing_value_for_cal[12]);
	printf("LOGIC_VO = %x\r\n", adc_sensing_value_for_cal[17]);
	printf("/////////////////////////////\r\n");
	printf("AVDD = %x\r\n", adc_sensing_value_for_cal[1]);
	printf("VDD8_S = %x\r\n", adc_sensing_value_for_cal[6]);	
	printf("VDD8_G = %x\r\n", adc_sensing_value_for_cal[11]);	
	printf("VGH = %x\r\n", adc_sensing_value_for_cal[16]);	
	printf("VGL = %x\r\n", adc_sensing_value_for_cal[21]);	
	printf("VINIT = %x\r\n", adc_sensing_value_for_cal[26]);
	printf("AP_SPARE1 = %x\r\n", adc_sensing_value_for_cal[31]);
	printf("AP_SPARE2 = %x\r\n", adc_sensing_value_for_cal[36]);
	printf("/////////////////////////////\r\n");	
	printf("VDD11 = %x\r\n", adc_sensing_value_for_cal[0]);
	printf("VDD18 = %x\r\n", adc_sensing_value_for_cal[5]);	
	printf("DP_SPARE1 = %x\r\n", adc_sensing_value_for_cal[10]);	
	printf("DP_SPARE2 = %x\r\n", adc_sensing_value_for_cal[15]);
	printf("VDD = %x\r\n", adc_sensing_value_for_cal[20]);	
	printf("VSS = %x\r\n", adc_sensing_value_for_cal[25]);	
}
void ADC_DATA_PRINT(void)
{
	printf("LED_ELVDD = %d\r\n", adc_sensing_value[4]);
	printf("LDO_OSC = %d\r\n", adc_sensing_value[9]);
	printf("LDO_VGH = %d\r\n", adc_sensing_value[14]);
	printf("LDO_VGL = %d\r\n", adc_sensing_value[19]);
	printf("LDO_VINT = %d\r\n", adc_sensing_value[24]);
	printf("VCIR = %d\r\n", adc_sensing_value[29]);
	printf("VREF1 = %d\r\n", adc_sensing_value[34]);
	printf("VREG1 = %d\r\n", adc_sensing_value[39]);
	printf("VOTP50 = %d\r\n", adc_sensing_value[3]);
	printf("PM_SPARE1 = %d\r\n", adc_sensing_value[8]);
	printf("/////////////////////////////\r\n");	
	printf("MON1 = %d\r\n", adc_sensing_value[13]);		
	printf("MON2 = %d\r\n", adc_sensing_value[18]);
	printf("MON3 = %d\r\n", adc_sensing_value[23]);
	printf("MON4 = %d\r\n", adc_sensing_value[28]);
	printf("MON5 = %d\r\n", adc_sensing_value[33]);
	printf("MON6 = %d\r\n", adc_sensing_value[38]);
	printf("LM_SPARE1 = %d\r\n", adc_sensing_value[2]);
	printf("LM_SPARE2 = %d\r\n", adc_sensing_value[7]);	
	printf("/////////////////////////////\r\n");
	printf("IVDD = %d\r\n", adc_sensing_value[22]);
	printf("IVSS = %d\r\n", adc_sensing_value[32]);
	printf("IVDD_100mA = %d\r\n", adc_sensing_value[27]);
	printf("IVSS_100mA= %d\r\n", adc_sensing_value[37]);	
	printf("/////////////////////////////\r\n");	
	printf("I2C_VO = %d\r\n", adc_sensing_value[12]);
	printf("LOGIC_VO = %d\r\n", adc_sensing_value[17]);
	printf("/////////////////////////////\r\n");
	printf("ELVDD = %d\r\n", adc_sensing_value[1]);
	printf("VDD8_S = %d\r\n", adc_sensing_value[6]);	
	printf("VDD8_G = %d\r\n", adc_sensing_value[11]);	
	printf("VGH = %d\r\n", adc_sensing_value[16]);	
	printf("VGL = %d\r\n", adc_sensing_value[21]);	
	printf("VINIT = %d\r\n", adc_sensing_value[26]);
	printf("AP_SPARE1 = %d\r\n", adc_sensing_value[31]);
	printf("AP_SPARE2 = %d\r\n", adc_sensing_value[36]);
	printf("/////////////////////////////\r\n");	
	printf("VDD11 = %d\r\n", adc_sensing_value[0]);
	printf("VDD18 = %d\r\n", adc_sensing_value[5]);	
	printf("DP_SPARE1 = %d\r\n", adc_sensing_value[10]);	
	printf("DP_SPARE2 = %d\r\n", adc_sensing_value[15]);
	printf("AVDD = %d\r\n", adc_sensing_value[20]);	
	printf("VSS = %d\r\n", adc_sensing_value[25]);																																		
}

void resistance_measurement_1k(void)
{
	int i = 0;
	int j = 0;
	int ready = 0;
	char l_r=0;
	unsigned int adc_read_data_0;
	unsigned int adc_read_data_1;
	float real_read_data_0;
	float real_read_data_1;
	float adc_data_0_a;	
	float adc_data_1_r;								
	unsigned short crc;	

	gpio->GPIO_DATA = 0x1cc;	//1Kohm Resistance Sel
	usleep(10);
	ads124s08_rm->ADC_AUTO_DELAY = 1000;

	ads124s08_rm->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE | ADC_Control_AUTO_READ_CLEAR;
	while(1)
	{
		if((ads124s08_rm->ADC_STATUS & (1<<2)))
		{
			ready = 1;
			break;
		}				
		usleep(1000);
		j++;
		//if(j > 5) return 0;
		if(j > 2000) break;
	}

	if(ready)	
	{
		for(int i = 0; i<2 ; i++)
		{
			if(i==0)	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x01;	
			else	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x03;	
			usleep(10);		
			adc_read_data_0 = ads124s08_rm->ADC_AUTO_DATA;						
			crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
			if(!crc)
			{
				if(adc_read_data_0 & 0x80000000)real_read_data_0 = ((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT;
				else real_read_data_0 = ((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT;						
				
				adc_read_data_0 = (adc_read_data_0 >>8);
				//printf("crc_rm_ok = %x / %f\r\n", adc_read_data_0, real_read_data_0);
			}
			else	printf("crc_rm_ng\r\n");

			if(i==0)	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x00;	
			else	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x02;		
			usleep(10);	
			adc_read_data_1 = ads124s08_rm->ADC_AUTO_DATA;
			crc = ADC_CRC8((unsigned char *)&adc_read_data_1, sizeof(adc_read_data_1));	 
			if(!crc)
			{
				if(adc_read_data_1 & 0x80000000)real_read_data_1 = ((float)((((~adc_read_data_1) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT;
				else real_read_data_1 = ((float)((adc_read_data_1 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT;						
				
				adc_read_data_1 = (adc_read_data_1 >>8);
				//printf("crc_rm[0]_ok = %x / %f\r\n", adc_read_data_1, real_read_data_1);
			}
			else	printf("crc_rm[0]_ng\r\n"); 	

			adc_data_0_a = real_read_data_0/1000;
			adc_data_1_r = real_read_data_1 / adc_data_0_a;
			if(i==0)	
			{
				//reg_value.adc_res_value_r_1k = adc_data_1_r;
				//printf("VR[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %f\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, reg_value.adc_res_value_r_1k);	
			}
			else	
			{
				//reg_value.adc_res_value_l_1k = adc_data_1_r;
				//printf("VL[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %f\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, reg_value.adc_res_value_l_1k);	
			}
			//printf("------------------------------------------------------\r\n");	
			//printf("V[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %f\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_data_1_r-ADC_REG_OFFSET);	
		}
	}
	else 	printf("TP2\r\n");
}

void resistance_measurement_1(void)
{
	int i = 0;
	int j = 0;
	int ready = 0;
	char l_r=0;
	unsigned int adc_read_data_0;
	unsigned int adc_read_data_1;
	double real_read_data_0;
	double real_read_data_1;
	double adc_data_0_a;	
	double adc_data_1_r;								
	unsigned short crc;	

	gpio->GPIO_DATA |= 0xaa;	//1ohm Resistance Sel
	//gpio->GPIO_DATA = 0x1aa;	//1ohm Resistance Sel
	usleep(1000);
	ads124s08_rm->ADC_AUTO_DELAY = 1000;

	if(ads124s08_rm->ADC_STATUS & (ADC_STATUS_AUTO_READ_DONE)) ads124s08_rm->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE | ADC_Control_AUTO_READ_CLEAR;
	else ads124s08_rm->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE;

	while(1)
	{
		if((ads124s08_rm->ADC_STATUS & (1<<2)))
		{
			ready = 1;
			break;
		}				
		usleep(1000);
		j++;
		//if(j > 5) return 0;
		if(j > 3000) break;
	}
	usleep(1000);
	if(ready)	
	{
		for(int i = 0; i<2 ; i++)
		{
			if(i==0)	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x01;	
			else	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x03;			
			adc_read_data_0 = ads124s08_rm->ADC_AUTO_DATA;						
			crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
			if(!crc)
			{
				if(adc_read_data_0 & 0x80000000)real_read_data_0 = ((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT;
				else real_read_data_0 = ((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT;						
				
				adc_read_data_0 = (adc_read_data_0 >>8);
				//printf("crc_rm_ok = %x / %f\r\n", adc_read_data_0, real_read_data_0);
			}
			else	printf("crc_rm_ng\r\n");

			if(i==0)	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x00;	
			else	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x02;		

			adc_read_data_1 = ads124s08_rm->ADC_AUTO_DATA;
			crc = ADC_CRC8((unsigned char *)&adc_read_data_1, sizeof(adc_read_data_1));	 
			if(!crc)
			{
				if(adc_read_data_1 & 0x80000000)real_read_data_1 = ((float)((((~adc_read_data_1) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT;
				else real_read_data_1 = ((float)((adc_read_data_1 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT;						
				
				adc_read_data_1 = (adc_read_data_1 >>8);
				//printf("crc_rm[0]_ok = %x / %f\r\n", adc_read_data_1, real_read_data_1);
			}
			else	printf("crc_rm[0]_ng\r\n"); 	

			adc_data_0_a = real_read_data_0/1;
			adc_data_1_r = real_read_data_1 / adc_data_0_a;
			if(i==0)	
			{
				adc_res_value_r_1 = (int)((adc_data_1_r-ADC_REG_R_OFFSET)*10000);
				//if(adc_res_value_r_1 > 3000000)	adc_res_value_r_1 = 0xffffffff;
				if(cprf) printf("1ohm_VR[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %d\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_res_value_r_1);	
			}
			else	
			{
				adc_res_value_l_1 = (int)((adc_data_1_r-ADC_REG_L_OFFSET)*10000);
				//if(adc_res_value_l_1 > 3000000)	adc_res_value_l_1 = 0xffffffff;
				if(cprf) printf("1ohm_VL[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %d\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_res_value_l_1);	
			}
			//printf("------------------------------------------------------\r\n");	
			//printf("V[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %f\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_res_value_l_1);	
		}
	}
	else 	printf("TP2\r\n");
}

void ads124_cal_load()
{
    FILE *adc_file;
	int i = 0;
    memset(&ads124_cal0, 0, sizeof(ads124_cal0));
    memset(&ads124_cal1, 0, sizeof(ads124_cal1));
    memset(&ads124_cal2, 0, sizeof(ads124_cal2));
    memset(&ads124_cal3, 0, sizeof(ads124_cal3));
    memset(&ads124_cal4, 0, sizeof(ads124_cal4));  
	memset(&avdd_cur_cal, 0, sizeof(avdd_cur_cal));   
	memset(&elvss_cur_cal, 0, sizeof(elvss_cur_cal)); 
    usleep(1000);   

    if(system("ls /f0/config/ads124_cal0.info"))
    {
       fopen(ADS124_0_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_0_CAL_FILE_PATH, "r");
        fread(&ads124_cal0, sizeof(ads124_cal0), 1, adc_file);  
        usleep(1000);
    }

    if(system("ls /f0/config/ads124_cal1.info"))
    {
       fopen(ADS124_1_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_1_CAL_FILE_PATH, "r");
        fread(&ads124_cal1, sizeof(ads124_cal1), 1, adc_file);  
        usleep(1000);
    }

    if(system("ls /f0/config/ads124_cal2.info"))
    {
       fopen(ADS124_2_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_2_CAL_FILE_PATH, "r");
        fread(&ads124_cal2, sizeof(ads124_cal2), 1, adc_file);  
        usleep(1000);
    }

    if(system("ls /f0/config/ads124_cal3.info"))
    {
       fopen(ADS124_3_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_3_CAL_FILE_PATH, "r");
        fread(&ads124_cal3, sizeof(ads124_cal3), 1, adc_file);  
        usleep(1000);
    }

    if(system("ls /f0/config/ads124_cal4.info"))
    {
       fopen(ADS124_4_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_4_CAL_FILE_PATH, "r");
        fread(&ads124_cal4, sizeof(ads124_cal4), 1, adc_file);  
        usleep(1000);
    }

    if(system("ls /f0/config/avdd_cur_cal.info"))
    {
       fopen(AVDD_CUR_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(AVDD_CUR_CAL_FILE_PATH, "r");
        fread(&avdd_cur_cal, sizeof(avdd_cur_cal), 1, adc_file);  
        usleep(1000);
    }	

    if(system("ls /f0/config/elvss_cur_cal.info"))
    {
       fopen(ELVSS_CUR_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ELVSS_CUR_CAL_FILE_PATH, "r");
        fread(&elvss_cur_cal, sizeof(elvss_cur_cal), 1, adc_file);  
        usleep(1000);
    }

	printf("ads124_cal0.adc_15v_value = %f / ads124_cal0.adc_0v_value = %f\r\n", ads124_cal0.adc_15v_value, ads124_cal0.adc_0v_value);
	printf("ads124_cal0.adc_15v_step = %f / ads124_cal0.adc_0v_step = %f\r\n", ads124_cal0.adc_15v_step, ads124_cal0.adc_0v_step);
	printf("ads124_cal0.adc_ratio = %f / ads124_cal0.offset = %f\r\n", ads124_cal0.adc_p_ratio, ads124_cal0.adc_p_offset);

	printf("ads124_cal1.adc_15v_value = %f / ads124_cal1.adc_0v_value = %f\r\n", ads124_cal1.adc_15v_value, ads124_cal1.adc_0v_value);
	printf("ads124_cal1.adc_15v_step = %f / ads124_cal1.adc_0v_step = %f\r\n", ads124_cal1.adc_15v_step, ads124_cal1.adc_0v_step);
	printf("ads124_cal1.adc_ratio = %f / ads124_cal1.offset = %f\r\n", ads124_cal1.adc_p_ratio, ads124_cal1.adc_p_offset);

	printf("ads124_cal2.adc_15v_value = %f / ads124_cal2.adc_0v_value = %f\r\n", ads124_cal2.adc_15v_value, ads124_cal2.adc_0v_value);
	printf("ads124_cal2.adc_15v_step = %f / ads124_cal2.adc_0v_step = %f\r\n", ads124_cal2.adc_15v_step, ads124_cal2.adc_0v_step);
	printf("ads124_cal2.adc_ratio = %f / ads124_cal2.offset = %f\r\n", ads124_cal2.adc_p_ratio, ads124_cal2.adc_p_offset);

	printf("ads124_cal3.adc_15v_value = %f / ads124_cal3.adc_0v_value = %f\r\n", ads124_cal3.adc_15v_value, ads124_cal3.adc_0v_value);
	printf("ads124_cal3.adc_15v_step = %f / ads124_cal3.adc_0v_step = %f\r\n", ads124_cal3.adc_15v_step, ads124_cal3.adc_0v_step);
	printf("ads124_cal3.adc_ratio = %f / ads124_cal3.offset = %f\r\n", ads124_cal3.adc_p_ratio, ads124_cal3.adc_p_offset);		

	printf("ads124_cal4.adc_15v_value = %f / ads124_cal4.adc_0v_value = %f\r\n", ads124_cal4.adc_15v_value, ads124_cal4.adc_0v_value);
	printf("ads124_cal4.adc_15v_step = %f / ads124_cal4.adc_0v_step = %f\r\n", ads124_cal4.adc_15v_step, ads124_cal4.adc_0v_step);
	printf("ads124_cal4.adc_ratio = %f / ads124_cal4.offset = %f\r\n", ads124_cal4.adc_p_ratio, ads124_cal4.adc_p_offset);	

	printf("avdd_cur_cal.cur_100ma_value = %f / avdd_cur_cal.cur_0ma_value = %f\r\n", avdd_cur_cal.cur_100ma_value, avdd_cur_cal.cur_0ma_value);
	printf("avdd_cur_cal.cur_100ma_step = %f / avdd_cur_cal.cur_0ma_step = %f\r\n", avdd_cur_cal.cur_100ma_step, avdd_cur_cal.cur_0ma_step);
	printf("avdd_cur_cal.cur_100ma_ratio = %f / avdd_cur_cal.cur_100ma_offset = %f\r\n", avdd_cur_cal.cur_100ma_ratio, avdd_cur_cal.cur_100ma_offset);		
	printf("avdd_cur_cal.cur_100ma_user_offset = %f\r\n", avdd_cur_cal.cur_100ma_user_offset);

	printf("avdd_cur_cal.cur_1a_value = %f / avdd_cur_cal.cur_0ma_value = %f\r\n", avdd_cur_cal.cur_1a_value, avdd_cur_cal.cur_0a_value);
	printf("avdd_cur_cal.cur_1a_step = %f / avdd_cur_cal.cur_0ma_step = %f\r\n", avdd_cur_cal.cur_1a_step, avdd_cur_cal.cur_0a_step);
	printf("avdd_cur_cal.cur_1a_ratio = %f / avdd_cur_cal.cur_1a_offset = %f\r\n", avdd_cur_cal.cur_1a_ratio, avdd_cur_cal.cur_1a_offset);		
	printf("avdd_cur_cal.cur_1a_user_offset = %f\r\n", avdd_cur_cal.cur_1a_user_offset);	

	printf("elvss_cur_cal.cur_100ma_value = %f / elvss_cur_cal.cur_0ma_value = %f\r\n", elvss_cur_cal.cur_100ma_value, elvss_cur_cal.cur_0ma_value);
	printf("elvss_cur_cal.cur_100ma_step = %f / elvss_cur_cal.cur_0ma_step = %f\r\n", elvss_cur_cal.cur_100ma_step, elvss_cur_cal.cur_0ma_step);
	printf("elvss_cur_cal.cur_100ma_ratio = %f / elvss_cur_cal.cur_100ma_offset = %f\r\n", elvss_cur_cal.cur_100ma_ratio, elvss_cur_cal.cur_100ma_offset);		
	printf("elvss_cur_cal.cur_100ma_user_offset = %f\r\n", elvss_cur_cal.cur_100ma_user_offset);

	printf("elvss_cur_cal.cur_1a_value = %f / elvss_cur_cal.cur_0a_value = %f\r\n", elvss_cur_cal.cur_1a_value, elvss_cur_cal.cur_0a_value);
	printf("elvss_cur_cal.cur_1a_step = %f / elvss_cur_cal.cur_0a_step = %f\r\n", elvss_cur_cal.cur_1a_step, elvss_cur_cal.cur_0a_step);
	printf("elvss_cur_cal.cur_1a_ratio = %f / elvss_cur_cal.cur_1a_offset = %f\r\n", elvss_cur_cal.cur_1a_ratio, elvss_cur_cal.cur_1a_offset);		
	printf("elvss_cur_cal.cur_1a_user_offset = %f\r\n", elvss_cur_cal.cur_1a_user_offset);						
}

char adc_sen_monitoring(char *cmd, short data)
{
	int i = 0;
	int j = 0;
	int sesing_code = -1;
	char done = 0;
    const char *sensing_list[MAX_SENSING_COUNT] = {"DP0","AP0","LM6","PM8","PM0","DP1","AP1","LM7","PM9","PM1","DP2","AP2","I2C","LM0","PM2",
    "DP3", "AP3", "LOGIC", "LM1", "PM3", "ELVDD", "AP4", "IDD0", "LM2", "PM4", "ELVSS", "AP5", "IDD1", "LM3", "PM5", "NULL0","AP6","ISS0", 
	"LM4", "PM6", "NULL1","AP7", "ISS1", "LM5", "PM7"};	
	
	for(i = 0 ; i < MAX_SENSING_COUNT ; i++)
	{
		if(!strcasecmp(cmd,sensing_list[i]))
		{
			sesing_code=i;	
			break;
		}				
	}
	
	while(1)
	{
		ADC_AUTO_DATA_READ();
		if((adc_sensing_value[sesing_code])> data)
		{
			done = 1;
			break;
		}				
		usleep(1000);
		j++;
		//if(j > 5) return 0;
		if(j > 50) break;
	}

	if(done)	return 1;
	else 	
	{
		printf("Monitoring_Sensing_Fail Sensing_data = %d / Criteria = %d\r\n", adc_sensing_value[sesing_code], data);
		return 0;
	}
}

