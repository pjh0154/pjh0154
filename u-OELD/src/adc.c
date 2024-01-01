#include "../include/adc.h"
#include "../include/fpga_reg.h"
#include "../include/ep.h"
#include "../include/application.h"

extern int cprf;
extern int rprf;
extern int nprf;

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
			//*init_value = 0x38; //about 440ms	
			//*init_value = 0x23; //about 440ms
			//*init_value = 0x28; //about 440ms
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

//static int ADC_RM_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS_NUM address, unsigned char *init_value)
int ADC_RM_REG_SETTING_VALUE_LOAD(ADC_REG_ADDRESS_NUM address, unsigned char *init_value)
{
	switch(address)
	{
		case ID:
			return 0;
		case STATUS:
			*init_value = 0x00;
			rm_init_data[STATUS_ADDR]= 0x00;
			return 1;
		case PGA:
			//*init_value = 0x01;
			*init_value = 0x00;
			rm_init_data[PGA_ADDR] =	0x00; 
			//PGA_GAIN x2
			return 1;			
		case DATARATE:
			//*init_value = 0x3D;
			//*init_value = 0x39; //about 160ms
			*init_value = 0x29; //about 440ms
			//*init_value = 0x25; //about 440ms
			rm_init_data[DATARATE_ADDR] =	0x29; 			
			return 1;
		case REF:
			*init_value = 0x10;
			    rm_init_data[REF_ADDR] =	0x10; 
			//*init_value = 0x1A;
			return 1;
		case SYS:
			*init_value = 0x12;
			rm_init_data[SYS_ADDR] =	0x12; 
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

void ads124s08_short_check_set(int address, int data)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data; 
	int adc_start = 0;	//231015 Modify
	int adc_end = 0;	//231015 Modify
	int adc_time = ADC_TIMEOUT;	//231015 Modify	   
  
	i = 4;
	
	ads124s08_sensing->ADC_REG_ADDRESS = address;
	ads124s08_sensing->ADC_REG_WRITE = data;
	ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_WRITE; 
	j=0;
	ready = 0;	
	adc_start = timeout_msclock();	//231015 Modify		
	while(1)
	{
		if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_WRITE))
		{
			ready = 1;
			break;
		}				
		//usleep(5000);
		//usleep(1);
		//j++;
		//if(j > 20000000) break;
		usleep(1);	//231013 Modify
		adc_end = timeout_msclock() - adc_start;	//231015 Modify
		if(adc_end >= adc_time) break;	//231015 Modify			
	}
	if(!ready)	
	{
		printf("ADS124_WRITE_FAIL\r\n");
		return;
	} 
	if(rprf)	
	{				          
		for(i = 0 ; i <= 18 ; i++)
		{
			ads124s08_sensing->ADC_REG_ADDRESS = i;
			ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_READ;
			j=0;
			ready = 0;
			adc_start = 0;	//231015 Modify
			adc_end = 0;	//231015 Modify	
			adc_start = timeout_msclock();	//231015 Modify						
			while(1)
			{
				if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_READ))
				{
					ready = 1;
					break;
				}				
				//usleep(5000);
				//j++;
				//usleep(1);
				//if(j > 20000000) break;
				usleep(1);	//231013 Modify
				adc_end = timeout_msclock() - adc_start;	//231015 Modify
				if(adc_end >= adc_time) break;	//231015 Modify					
			}
			if(!ready)	
			{
				printf("ADS124_READ_FAIL\r\n");
				return;
			} 		
			printf("REG%d = %x\r\n", i, ads124s08_sensing->ADC_REG_READ);
			if(i == 18)printf("------------\r\n");
		}
    }	
}

void ads124s08_sensing_init(void)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data;    
	int adc_init_start = 0;	//231015 Modify
	int adc_init_end = 0;	//231015 Modify
	int adc_init_time = ADC_TIMEOUT;	//231015 Modify

    ads124s08_sensing->ADC_CONTROL = ADC_Control_RESET;
	adc_init_start = timeout_msclock();	//231015 Modify
	while(1)
	{
		if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_RESET))
		{
			ready = 1;
			break;
		}				
		usleep(1);	//231013 Modify
		adc_init_end = timeout_msclock() - adc_init_start;	//231015 Modify
		if(adc_init_end >= adc_init_time) break;	//231015 Modify				
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
		adc_init_start = 0;	//231015 Modify
		adc_init_end = 0;	//231015 Modify	
		adc_init_start = timeout_msclock();	//231015 Modify	
		while(1)
		{
			if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(1);	//231015 Modify
			adc_init_end = timeout_msclock() - adc_init_start;	//231015 Modify
			if(adc_init_end >= adc_init_time) break;	//231015 Modify				
		}	
		if(!ready)	
		{
			printf("ADS124_READ_FAIL\r\n");
			return;
		} 
        printf("REG%d = %x\r\n", i, ads124s08_sensing->ADC_REG_READ);
		usleep(10000);
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
			adc_init_start = 0;	//231015 Modify
			adc_init_end = 0;	//231015 Modify	
			adc_init_start = timeout_msclock();	//231015 Modify							
			while(1)
			{
				if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_WRITE))
				{
					ready = 1;
					break;
				}				
				usleep(1);	//231015 Modify
				adc_init_end = timeout_msclock() - adc_init_start;	//231015 Modify
				if(adc_init_end >= adc_init_time) break;	//231015 Modify						
			}
			if(!ready)	
			{
				printf("ADS124_WRITE_FAIL\r\n");
				return;
			} 					          
        } 
		usleep(10000);
    }
    printf("------------------------------------------\r\n");
    for(i = 0 ; i <= 18 ; i++)
    {
        ads124s08_sensing->ADC_REG_ADDRESS = i;
        ads124s08_sensing->ADC_CONTROL = ADC_Control_REG_READ;
		j=0;
		ready = 0;	
		adc_init_start = 0;	//231015 Modify
		adc_init_end = 0;	//231015 Modify	
		adc_init_start = timeout_msclock();	//231015 Modify					
		while(1)
		{
			if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_READ))
			{
				ready = 1;
				break;
			}				
			usleep(1);	//231015 Modify
			adc_init_end = timeout_msclock() - adc_init_start;	//231015 Modify
			if(adc_init_end >= adc_init_time) break;	//231015 Modify					
		}
		if(!ready)	
		{
			printf("ADS124_READ_FAIL\r\n");
			return;
		} 		
        //data = ads124s08_sensing->ADC_REG_READ;
        printf("REG%d = %x\r\n", i, ads124s08_sensing->ADC_REG_READ);
		usleep(10000);
    }
    //ads124s08_sensing->ADC_CONTROL = ADC_Control_START_PIN_HIGH;     
}

void ads124s08_ads124s08_rm_init(void)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data;    
	int rm_init_start = 0;	//231015 Modify
	int rm_init_end = 0;	//231015 Modify
	int rm_init_time = ADC_TIMEOUT;	//231015 Modify
	int rm_init_error_time_delay= 5000;
	int cnt = 0;
	unsigned char ads_rm_init_result = 0;
	unsigned int init_data = 0;
	rm_error_cnt_result = 0;
	rm_error_ready_result = 0;
	ads_rm_init_data_set_task();
    ads124s08_rm->ADC_CONTROL = ADC_Control_RESET;
	rm_init_start = timeout_msclock();	//231015 Modify
	while(1)
	{
		if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_RESET))
		{
			ready = 1;
			break;
		}				
		usleep(1);	//231013 Modify
		rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
		if(rm_init_end >= rm_init_time) break;	//231015 Modify			
	}
	usleep(50000);
	if(!ready)	
	{
		rm_error_ready_result = RM_ADC_NG;
		printf("----------ADS124_RESET_FAIL----------\r\n");
		return;			
	}
    printf("----------ads124s08_rm_init----------\r\n");	  
    //for(i = 0 ; i <= 18 ; i++)
	for(cnt = 0; cnt < 3 ; cnt++)
	{
		for(i = 0 ; i < 18 ; i++)
		{
			ads124s08_rm->ADC_REG_ADDRESS = i;
			//ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
			ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
			j=0;
			ready = 0;
			rm_init_start = 0;	//231015 Modify
			rm_init_end = 0;	//231015 Modify
			rm_init_start = timeout_msclock();	//231015 Modify		
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
				{
					ready = 1;
					break;
				}				
				usleep(1);	//231015 Modify
				rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
				if(rm_init_end >= rm_init_time) break;	//231015 Modify				
			}
			if(!ready)	
			{
				rm_error_ready_result = RM_ADC_NG;
				printf("----------ADS124_RM_READ_FAIL----------\r\n");		
				return;							
			}				
			//data = ads124s08_sensing->ADC_REG_READ;
			if(cnt == 2)	printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
			init_data =	ads124s08_rm->ADC_REG_READ; 
			ads_rm_init_result |= ads_rm_init_data_compare_task(i,init_data);
			usleep(10000);	
		}			
	}	
	if(ads_rm_init_result)	//231117 Modify	
	{
		rm_error_cnt_result = RM_ADC_NG;		
		printf("ADS_RM INIT PROBLEM OCCURRED!\r\n");
		printf("Turn off the power switch and turn it b ack on\r\n");	
		return;	
	}		
    printf("-------------------------------------\r\n");
    //for(i = 0 ; i <= 18 ; i++)
	for(i = 0 ; i < 18 ; i++)
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
			rm_init_start = 0;	//231015 Modify
			rm_init_end = 0;	//231015 Modify
			rm_init_start = timeout_msclock();	//231015 Modify				
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_WRITE))
				{
					ready = 1;
					break;
				}				
				usleep(1);	//231015 Modify
				rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
				if(rm_init_end >= rm_init_time) break;	//231015 Modify						
			}
			if(!ready)	
			{		
				rm_error_ready_result = RM_ADC_NG;				
				printf("----------ADS124_RM_WRITE_FAIL----------\r\n");		
				return;										
			}							          
        } 
		usleep(10000);	
    }	
    printf("-------------------------------------\r\n");
    //for(i = 0 ; i <= 18 ; i++)
	for(cnt = 0; cnt < 3 ; cnt++)
	{	
		for(i = 0 ; i < 18 ; i++)
		{
			ads124s08_rm->ADC_REG_ADDRESS = i;
			//ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
			ads124s08_rm->ADC_CONTROL = ADC_Control_REG_READ;
			j=0;
			ready = 0;
			rm_init_start = 0;	//231015 Modify
			rm_init_end = 0;	//231015 Modify
			rm_init_start = timeout_msclock();	//231015 Modify			
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_READ))
				{
					ready = 1;
					break;
				}				
				usleep(1);	//231015 Modify
				rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
				if(rm_init_end >= rm_init_time) break;	//231015 Modify					
			}
			if(!ready)	
			{
				rm_error_ready_result = RM_ADC_NG;				
				printf("----------ADS124_RM_READ_FAIL----------\r\n");	
				return;														
			}			
			//data = ads124s08_sensing->ADC_REG_READ;
			if(cnt == 2)	printf("REG%d = %x\r\n", i, ads124s08_rm->ADC_REG_READ);
			init_data =	ads124s08_rm->ADC_REG_READ; 
			ads_rm_init_result |= ads_rm_init_data_compare_task(i,init_data);
			usleep(10000);				
		}
	}
	if(ads_rm_init_result)	//231117 Modify	
	{
		rm_error_cnt_result = RM_ADC_NG;		
		printf("ADS_RM INIT PROBLEM OCCURRED!\r\n");
		printf("Turn off the power switch and turn it b ack on\r\n");
		return;	
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
	data_ptr=NULL;	
	return crc; 
}

//void ADC_AUTO_DATA_READ(void)
unsigned char ADC_AUTO_DATA_READ(void)
{
	int i = 0;
	int j = 0;
	int ii = 0;
	int iii = 0;
	int ready = 0;
	int mux_sel = 7;
	int adc_auto_start = 0;	//231015 Modify
	int adc_auto_end = 0;	//231015 Modify
	int adc_auto_time = ADC_TIMEOUT;	//231015 Modify
	unsigned int adc_read_data_0 = 0;
	float real_read_data_0 = 0;
	unsigned int adc_read_data_1 = 0;
	float real_read_data_1 = 0;
	float adc_data_0_a = 0;	
	float adc_data_1_r = 0;								
	unsigned short crc = 0;	
	unsigned char adc_result = ADC_RESULT_OK;
	unsigned char index = 0;

	ads124s08_sensing->ADC_AUTO_DELAY = 2000;						
	memset(&adc_sensing_value, 0, sizeof(adc_sensing_value));
	if((ads124s08_sensing->ADC_STATUS & ADC_STATUS_AUTO_READ_DONE))	ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE | ADC_Control_AUTO_READ_CLEAR;	
	else ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE;

	adc_auto_start = timeout_msclock();	//231015 Modify
	while(1)
	{
		if((ads124s08_sensing->ADC_STATUS & ADC_STATUS_AUTO_READ_DONE))
		{
			ready = 1;
			break;
		}			
		//usleep(1000);
		//j++;
		//if(j > 5) return 0;
		//if(j > 20000000) break;		
		usleep(1);
		adc_auto_end = timeout_msclock() - adc_auto_start;	//231015 Modify
		if(adc_auto_end >= adc_auto_time) break;	//231015 Modify			
	}
	usleep(1000);
	if(ready)	
	{
		for(ii=39 ; ii >= 0 ; ii--)
		{
			index = adc_auto_offset_select_task(ii);
			ads124s08_sensing->ADC_AUTO_DATA_SEL = ii;	
			adc_read_data_0 = ads124s08_sensing->ADC_AUTO_DATA;						
			crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
			if(!crc)
			{
				if((ii == ADC_IVDD ))
				{
					vdd_cur_offset_select();
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * avdd_cur_cal.cur_1a_ratio+avdd_cur_cal.cur_1a_offset)+avdd_cur_cal.n_cur_1a_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * avdd_cur_cal.cur_1a_ratio+avdd_cur_cal.cur_1a_offset)+avdd_cur_cal.cur_1a_user_offset);	
				}
				else if((ii ==ADC_IVSS ))
				{
					vss_cur_offset_select();					
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.n_cur_1a_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.cur_1a_user_offset);	
				}
				else if((ii ==ADC_IVDD_100mA))
				{
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
				}				
				usleep(1000);
				//adc_sensing_value[i] = real_read_data_0;
				if((ii == ADC_IVDD ) || (ii ==ADC_IVSS ))	
				{
					adc_sensing_value[i] = real_read_data_0*10;				
				}
				else if((ii ==ADC_IVDD_100mA ) || (ii ==ADC_IVSS_100mA ))
				{	
					adc_sensing_value[i] = real_read_data_0*100;						
				}

				else 
				{
					/*if((real_read_data_0 > 0) && (real_read_data_0 <= 10))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_10V]/1000);
					else if((real_read_data_0 > 10) && (real_read_data_0 <= 20))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_20V]/1000);
					else if((real_read_data_0 > 20) && (real_read_data_0 <= 25.5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_25V]/1000);
					else if((real_read_data_0 < 0) && (real_read_data_0 >= -10))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N10V]/1000);
					else if((real_read_data_0 < -10) && (real_read_data_0 >= -20))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N20V]/1000);	
					else if((real_read_data_0 < -20) && (real_read_data_0 >= -25.5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N25V]/1000);*/

					if((real_read_data_0 > 0) && (real_read_data_0 <= 5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_5V]/1000);
					else if((real_read_data_0 > 5) && (real_read_data_0 <=10))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_10V]/1000);
					else if((real_read_data_0 > 10) && (real_read_data_0 <=15))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_15V]/1000);
					else if((real_read_data_0 > 15) && (real_read_data_0 <= 20))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_20V]/1000);
					else if((real_read_data_0 > 20) && (real_read_data_0 <= 25.5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_25V]/1000);
					else if((real_read_data_0 < 0) && (real_read_data_0 >= -5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N5V]/1000);
					else if((real_read_data_0 < -5) && (real_read_data_0 >= -10))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N10V]/1000);
					else if((real_read_data_0 < -10) && (real_read_data_0 >= -15))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N15V]/1000);
					else if((real_read_data_0 < -15) && (real_read_data_0 >= -20))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N20V]/1000);	
					else if((real_read_data_0 < -20) && (real_read_data_0 >= -25.5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N25V]/1000);						

					adc_sensing_value[i] = real_read_data_0*1000;
				}
				i++;

			}
			else	
			{
				printf("crc_rm_ng\r\n");
				adc_result = ADC_RESULT_NG;
				error_data.vol_cur_adc = ERROR_NG;	//231013 Modify	
				ads124s08_sensing_init();	//231013 Modify	
			}
		}											
	}
	else
	{
		 printf("reday_ng\r\n");
		 error_data.vol_cur_adc = ERROR_NG;
		 ads124s08_sensing_init();		 
		 adc_result = ADC_RESULT_NG;
	}
	return adc_result;	
}

void ADC_SELECT_DATA_READ_AVG(unsigned char ch)
{
		int i = 0;
		int j = 0;
		int count = 0;
		int ready = 0;
		unsigned int adc_read_data_0 = 0;
		float real_read_data_0 = 0;
		//float read_data_avg[600] = {0};
		unsigned short crc = 0;
		int adc_start = 0;	//231015 Modify
		int adc_end = 0;	//231015 Modify
		int adc_time = ADC_TIMEOUT;	//231015 Modify
		unsigned char index = 0;
		index = adc_select_offset_select_task(ch);
		memset(&adc_sensing_value, 0, sizeof(adc_sensing_value));
		//ads124s08_sensing->ADC_AUTO_DELAY = 2000;	
		
		ads124s08_sensing->ADC_CONTROL |= ADC_Control_START_PIN_LOW;	
		ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_CLEAR;
		if((ch == SEN_VDD11) || (ch == SEN_ELVDD) || (ch == SEN_LM_SPARE1) || (ch == SEN_VOTP50) || (ch == SEN_LDO_ELVDD))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_0; 	
		else if((ch == SEN_VDD18) || (ch == SEN_ADD8_S) || (ch == SEN_LM_SPARE2) || (ch == SEN_PM_SPARE1) || (ch == SEN_LDO_OSC))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_1; 	
		else if((ch == SEN_DPSPARE1) || (ch == SEN_ADD8_G) || (ch == SEN_I2C) || (ch == SEN_MON1) || (ch == SEN_LDO_VGH))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_2; 
		else if((ch == SEN_DPSAPRE2) || (ch == SEN_VGH) || (ch == SEN_LOGIC) || (ch == SEN_MON2) || (ch == SEN_LDO_VGL))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_3; 
		else if((ch == SEN_AVDD) || (ch == SEN_VGL) || (ch == SEN_ELIDD) || (ch == SEN_MON3) || (ch == SEN_LDO_VINT))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_4;
		else if((ch == SEN_ELVSS) || (ch == SEN_VINT) || (ch == SEN_ELIDD_100mA) || (ch == SEN_MON4) || (ch == SEN_VCIR))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_5;		
		else if((ch == SEN_APSPARE1) || (ch == SEN_ELISS) || (ch == SEN_MON5) || (ch == SEN_VREF1))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_6;
		else if((ch == SEN_APSPARE2) || (ch == SEN_ELISS_100mA) || (ch == SEN_MON6) || (ch == SEN_VREG1))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_7;

		if((ch == SEN_LDO_ELVDD) || (ch == SEN_LDO_OSC) || (ch == SEN_LDO_VGH) || (ch == SEN_LDO_VGL) || (ch == SEN_LDO_VINT) || (ch == SEN_VCIR) || (ch == SEN_VREF1) || (ch == SEN_VREG1))
		{
			ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN0 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
		}
		else if((ch == SEN_VOTP50) || (ch == SEN_PM_SPARE1) || (ch == SEN_MON1) || (ch == SEN_MON2) || (ch == SEN_MON3) || (ch == SEN_MON4) || (ch == SEN_MON5) || (ch == SEN_MON6))
		{
			ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN1 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));		
		}
		else if((ch == SEN_LM_SPARE1) || (ch == SEN_LM_SPARE2) || (ch == SEN_I2C) || (ch == SEN_LOGIC) || (ch == SEN_ELIDD) || (ch == SEN_ELIDD_100mA) || (ch == SEN_ELISS) || (ch == SEN_ELISS_100mA))
		{
			ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN2 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
		}
		else if((ch == SEN_ELVDD) || (ch == SEN_ADD8_S) || (ch == SEN_ADD8_G) || (ch == SEN_VGH) || (ch == SEN_VGL) || (ch == SEN_VINT) || (ch == SEN_APSPARE1) || (ch == SEN_APSPARE2))
		{
			ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN3 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
		}	
		else if((ch == SEN_VDD11) || (ch == SEN_VDD18) || (ch == SEN_DPSPARE1) || (ch == SEN_DPSAPRE2) || (ch == SEN_AVDD) || (ch == SEN_ELVSS))
		{
			ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN4 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
		}	
		ads124s08_sensing->ADC_CONTROL |= ADC_Control_START_PIN_HIGH;					
		ads124s08_sensing->ADC_REG_ADDRESS = 2;
		ads124s08_sensing->ADC_CONTROL |= ADC_Control_REG_WRITE;  	
																
		//j = 0;

		adc_start = timeout_msclock();	//231015 Modify
		while(1)
		{
			//if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_REG_WRITE))
			if((ads124s08_sensing->ADC_STATUS & (1<<1)))
			{
				ready = 1;
				break;
			}			
			//usleep(5000);
			//j++;
			//if(j > 5) return 0;
			//if(j > 10000000) break;
			usleep(1);
			adc_end = timeout_msclock() - adc_start;	//231015 Modify
			if(adc_end >= adc_time) break;	//231015 Modify				

		}
		if(!ready)	
		{
			error_data.vol_cur_adc = ERROR_NG;
			ads124s08_sensing_init();			
			printf("ADS124_WRITE_FAIL\r\n");
			return;
		}
		j = 0;	
		ready = 0;
		ads124s08_sensing->ADC_REG_ADDRESS = 2;
		ads124s08_sensing->ADC_CONTROL |= ADC_Control_DATA_READ;

		adc_start = 0;	//231015 Modify
		adc_end = 0;	//231015 Modify
		adc_start = timeout_msclock();	//231015 Modify			
		while(1)
		{
			if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_DATA_READ))
			{
				ready = 1;
				break;
			}				
			//usleep(5000);
			//j++;
			//if(j > 5) return 0;
			//if(j > 10000000) break;
			usleep(1);
			adc_end = timeout_msclock() - adc_start;	//231015 Modify
			if(adc_end >= adc_time) break;	//231015 Modify	

		}
		if(!ready)	
		{
			error_data.vol_cur_adc = ERROR_NG;
			ads124s08_sensing_init();			
			printf("ADS124_READ_FAIL\r\n");
			return;
		}
		else 
		{
			adc_read_data_0 = ads124s08_sensing->ADC_DATA_READ;
			crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
			if(!crc)
			{
				if((ch == SEN_ELIDD ))
				{
					vdd_cur_offset_select();
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * avdd_cur_cal.cur_1a_ratio+avdd_cur_cal.cur_1a_offset)+avdd_cur_cal.n_cur_1a_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * avdd_cur_cal.cur_1a_ratio+avdd_cur_cal.cur_1a_offset)+avdd_cur_cal.cur_1a_user_offset);	
				}			
				else if((ch ==SEN_ELISS ))
				{					
					vss_cur_offset_select();
					//if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.n_cur_1a_user_offset);
					//else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.cur_1a_user_offset);	
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.n_cur_1a_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.cur_1a_user_offset);										
				}
				/*else if((ch ==SEN_ELISS ))
				{					
					if(adc_read_data_0 & 0x80000000) 
					{
						if(((adc_read_data_0>>8) & 0x007FFFFF) < ADC_100mA_VALUE)	real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+n_elvss_cur_cal.n_cur_1a_user_offset);
						else real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * n_elvss_cur_cal.cur_1a_ratio+n_elvss_cur_cal.cur_1a_offset)+n_elvss_cur_cal.cur_1a_user_offset);
						//real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * n_elvss_cur_cal.cur_1a_ratio+n_elvss_cur_cal.cur_1a_offset)+n_elvss_cur_cal.cur_1a_user_offset);
					}
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_1a_ratio+elvss_cur_cal.cur_1a_offset)+elvss_cur_cal.cur_1a_user_offset);	
				}*/			
				else if((ch ==SEN_ELIDD_100mA))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * avdd_cur_cal.cur_100ma_ratio+avdd_cur_cal.cur_100ma_offset)+avdd_cur_cal.cur_100ma_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * avdd_cur_cal.cur_100ma_ratio+avdd_cur_cal.cur_100ma_offset)+avdd_cur_cal.cur_100ma_user_offset);											
				}
				else if((ch ==SEN_ELISS_100mA ))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)(((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * elvss_cur_cal.cur_100ma_ratio+elvss_cur_cal.cur_100ma_offset)+elvss_cur_cal.cur_100ma_user_offset);
					else real_read_data_0 = (((float)(((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * elvss_cur_cal.cur_100ma_ratio+elvss_cur_cal.cur_100ma_offset)+elvss_cur_cal.cur_100ma_user_offset);							
				}
				else if((ch == SEN_I2C ) || (ch == SEN_LOGIC ))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)*2);
					else real_read_data_0 = ((((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)*2);													
				}
				else if((ch == SEN_VDD11) || (ch == SEN_VDD18) || (ch == SEN_DPSPARE1) || (ch == SEN_DPSAPRE2) || (ch == SEN_AVDD) || (ch == SEN_ELVSS))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal4.adc_p_ratio+ads124_cal4.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal4.adc_p_ratio+ads124_cal4.adc_p_offset);							
				}
				else if((ch == SEN_ELVDD) || (ch == SEN_ADD8_S) || (ch == SEN_ADD8_G) || (ch == SEN_VGH) || (ch == SEN_VGL) || (ch == SEN_VINT) || (ch == SEN_APSPARE1) || (ch == SEN_APSPARE2))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal3.adc_p_ratio+ads124_cal3.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal3.adc_p_ratio+ads124_cal3.adc_p_offset);							
				}
				else if((ch == SEN_LM_SPARE1)||(ch == SEN_LM_SPARE2))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal2.adc_p_ratio+ads124_cal2.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal2.adc_p_ratio+ads124_cal2.adc_p_offset);	
				}
				else if((ch == SEN_VOTP50) || (ch == SEN_PM_SPARE1) || (ch == SEN_MON1) || (ch == SEN_MON2) || (ch == SEN_MON3) || (ch == SEN_MON4) || (ch == SEN_MON5) || (ch == SEN_MON6))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal1.adc_p_ratio+ads124_cal1.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal1.adc_p_ratio+ads124_cal1.adc_p_offset);							
				}
				else if((ch == SEN_LDO_ELVDD) || (ch == SEN_LDO_OSC) || (ch == SEN_LDO_VGH) || (ch == SEN_LDO_VGL) || (ch == SEN_LDO_VINT) || (ch == SEN_VCIR) || (ch == SEN_VREF1) || (ch == SEN_VREG1))
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * ads124_cal0.adc_p_ratio+ads124_cal0.adc_p_offset);
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * ads124_cal0.adc_p_ratio+ads124_cal0.adc_p_offset);							
				}												
				else
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = (((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT_1)*12;
					else real_read_data_0 = (((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT_1)*12;					
				}				
				//usleep(1000);
				//adc_sensing_value[i] = real_read_data_0;
				if((ch == SEN_ELIDD ) || (ch ==SEN_ELISS ))	
				{
					if(ch == SEN_ELIDD)	
					{
						avdd_cur_sensing_data = real_read_data_0;
					}
					else	
					{
						read_data_avg_tt = real_read_data_0; 
						elvss_cur_sensing_data = real_read_data_0;
						//printf("read_data_avg[%d] = %f\r\n", count, read_data_avg[count]);
					}
				}
				else if((ch ==SEN_ELIDD_100mA ) || (ch ==SEN_ELISS_100mA ))	
				{
					adc_sensing_value[ch] = real_read_data_0*100;
					if(ch == SEN_ELIDD_100mA)	cur_sensing_value[2] = real_read_data_0*1000;
					else	
					{
						cur_sensing_value[3] = (real_read_data_0*1000);					
						//printf("ELISS_100mA = %f\r\n", real_read_data_0+2.482519);
					}
				}	

				else 
				{
					if((real_read_data_0 > 0) && (real_read_data_0 <= 5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_5V]/1000);
					else if((real_read_data_0 > 5) && (real_read_data_0 <=10))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_10V]/1000);
					else if((real_read_data_0 > 10) && (real_read_data_0 <=15))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_15V]/1000);
					else if((real_read_data_0 > 15) && (real_read_data_0 <= 20))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_20V]/1000);
					else if((real_read_data_0 > 20) && (real_read_data_0 <= 25.5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_25V]/1000);
					else if((real_read_data_0 < 0) && (real_read_data_0 >= -5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N5V]/1000);
					else if((real_read_data_0 < -5) && (real_read_data_0 >= -10))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N10V]/1000);
					else if((real_read_data_0 < -10) && (real_read_data_0 >= -15))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N15V]/1000);
					else if((real_read_data_0 < -15) && (real_read_data_0 >= -20))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N20V]/1000);	
					else if((real_read_data_0 < -20) && (real_read_data_0 >= -25.5))	real_read_data_0 += (vol_offset[index].user_offset[ADC_CAL_N25V]/1000);

					adc_sensing_value[ch] = real_read_data_0*1000;
				}
			}
			else	
			{
				printf("crc_rm_ng\r\n");
				error_data.vol_cur_adc = ERROR_NG;	//231013 Modify	
				ads124s08_sensing_init();			//231013 Modify						
			}
			ads124s08_sensing->ADC_CONTROL &= ADC_SEL_PIN_7; 			
		} 

}

void ADC_AUTO_DATA_READ_FOR_CAL_N(void)
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
		//usleep(5000);
		j++;
		usleep(1);
		//if(j > 5) return 0;
		if(j > 10000000) break;
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
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1) * (float)(1.0);
					else real_read_data_0 = (((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0);							
				}
				else
				{
					if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1) * (float)(1.0);
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
		//usleep(5000);
		j++;
		usleep(1);
		//if(j > 5) return 0;
		if(j > 10000000) break;
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

void ADC_SELECT_DATA_READ_FOR_CAL(unsigned char ch)
{
	//int i = 0;
	//int j = 0;
	//int ii = 0;
	//int iii = 0;
	int ready = 0;
	int mux_sel = 7;
	unsigned int adc_read_data_0;
	int real_read_data_0;
	unsigned int adc_read_data_1;
	int real_read_data_1;
	int adc_data_0_a;	
	int adc_data_1_r;								
	unsigned short crc;
	int adc_start = 0;	
	int adc_end = 0;	
	int adc_time = ADC_TIMEOUT;	
	memset(&adc_sensing_value_for_cal, 0, sizeof(adc_sensing_value_for_cal));

	ads124s08_sensing->ADC_CONTROL |= ADC_Control_START_PIN_LOW;	
	ads124s08_sensing->ADC_CONTROL = ADC_Control_AUTO_READ_CLEAR;

	if((ch == SEN_VDD11) || (ch == SEN_ELVDD) || (ch == SEN_LM_SPARE1) || (ch == SEN_VOTP50) || (ch == SEN_LDO_ELVDD))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_0; 	
	else if((ch == SEN_VDD18) || (ch == SEN_ADD8_S) || (ch == SEN_LM_SPARE2) || (ch == SEN_PM_SPARE1) || (ch == SEN_LDO_OSC))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_1; 	
	else if((ch == SEN_DPSPARE1) || (ch == SEN_ADD8_G) || (ch == SEN_I2C) || (ch == SEN_MON1) || (ch == SEN_LDO_VGH))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_2; 
	else if((ch == SEN_DPSAPRE2) || (ch == SEN_VGH) || (ch == SEN_LOGIC) || (ch == SEN_MON2) || (ch == SEN_LDO_VGL))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_3; 
	else if((ch == SEN_AVDD) || (ch == SEN_VGL) || (ch == SEN_ELIDD) || (ch == SEN_MON3) || (ch == SEN_LDO_VINT))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_4;
	else if((ch == SEN_ELVSS) || (ch == SEN_VINT) || (ch == SEN_ELIDD_100mA) || (ch == SEN_MON4) || (ch == SEN_VCIR))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_5;		
	else if((ch == SEN_APSPARE1) || (ch == SEN_ELISS) || (ch == SEN_MON5) || (ch == SEN_VREF1))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_6;
	else if((ch == SEN_APSPARE2) || (ch == SEN_ELISS_100mA) || (ch == SEN_MON6) || (ch == SEN_VREG1))	ads124s08_sensing->ADC_CONTROL |= ADC_SEL_PIN_7;

	if((ch == SEN_LDO_ELVDD) || (ch == SEN_LDO_OSC) || (ch == SEN_LDO_VGH) || (ch == SEN_LDO_VGL) || (ch == SEN_LDO_VINT) || (ch == SEN_VCIR) || (ch == SEN_VREF1) || (ch == SEN_VREG1))
	{
		ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN0 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
	}
	else if((ch == SEN_VOTP50) || (ch == SEN_PM_SPARE1) || (ch == SEN_MON1) || (ch == SEN_MON2) || (ch == SEN_MON3) || (ch == SEN_MON4) || (ch == SEN_MON5) || (ch == SEN_MON6))
	{
		ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN1 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));		
	}
	else if((ch == SEN_LM_SPARE1) || (ch == SEN_LM_SPARE2) || (ch == SEN_I2C) || (ch == SEN_LOGIC) || (ch == SEN_ELIDD) || (ch == SEN_ELIDD_100mA) || (ch == SEN_ELISS) || (ch == SEN_ELISS_100mA))
	{
		ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN2 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
	}
	else if((ch == SEN_ELVDD) || (ch == SEN_ADD8_S) || (ch == SEN_ADD8_G) || (ch == SEN_VGH) || (ch == SEN_VGL) || (ch == SEN_VINT) || (ch == SEN_APSPARE1) || (ch == SEN_APSPARE2))
	{
		ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN3 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
	}	
	else if((ch == SEN_VDD11) || (ch == SEN_VDD18) || (ch == SEN_DPSPARE1) || (ch == SEN_DPSAPRE2) || (ch == SEN_AVDD) || (ch == SEN_ELVSS))
	{
		ads124s08_sensing->ADC_REG_WRITE = (((ADC_AIN4 << 4) & 0xf0) | (MUX_SEL_AINCOM & 0x0f));
	}	
	ads124s08_sensing->ADC_CONTROL |= ADC_Control_START_PIN_HIGH;					
	ads124s08_sensing->ADC_REG_ADDRESS = 2;
	ads124s08_sensing->ADC_CONTROL |= ADC_Control_REG_WRITE;  	

	adc_start = timeout_msclock();
	while(1)
	{
		if((ads124s08_sensing->ADC_STATUS & (1<<1)))
		{
			ready = 1;
			break;
		}			
		usleep(1);
		adc_end = timeout_msclock() - adc_start;	
		if(adc_end >= adc_time) break;			
	}
	if(!ready)	
	{
		error_data.vol_cur_adc = ERROR_NG;
		ads124s08_sensing_init();			
		printf("ADS124_WRITE_FAIL\r\n");
		return;
	}
	//j = 0;	
	ready = 0;
	ads124s08_sensing->ADC_REG_ADDRESS = 2;
	ads124s08_sensing->ADC_CONTROL |= ADC_Control_DATA_READ;

	adc_start = 0;	
	adc_end = 0;	
	adc_start = timeout_msclock();		
	while(1)
	{
		if(!(ads124s08_sensing->ADC_CONTROL&ADC_Control_DATA_READ))
		{
			ready = 1;
			break;
		}				
		usleep(1);
		adc_end = timeout_msclock() - adc_start;	
		if(adc_end >= adc_time) break;	
	}
	if(!ready)	
	{
		error_data.vol_cur_adc = ERROR_NG;
		ads124s08_sensing_init();			
		printf("ADS124_READ_FAIL\r\n");
		return;
	}

	else
	{		
		adc_read_data_0 = ads124s08_sensing->ADC_DATA_READ;			
		crc = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	 
		if(!crc)
		{
			if(adc_read_data_0 & 0x80000000) real_read_data_0 = ((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1) * (float)(-1.0);
			else real_read_data_0 = (((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0);	
			adc_sensing_value_for_cal[ch] = real_read_data_0;
		}
		else	printf("crc_rm_ng\r\n");
		ads124s08_sensing->ADC_CONTROL &= ADC_SEL_PIN_7; 														
	}	
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
	printf("LDO_ELVDD = %d\r\n", adc_sensing_value[4]);
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
	//printf("IVDD = %d\r\n", cur_sensing_value[0]);
	//printf("IVSS = %d\r\n", cur_sensing_value[1]);
	//printf("IVDD_100mA = %d\r\n", cur_sensing_value[2]);
	//printf("IVSS_100mA= %d\r\n", cur_sensing_value[3]);
	if(total_sen_cur_4byte_flag)
	{
		printf("IVDD_4BYTE = %d\r\n", cur_sensing_value[0]);
		printf("IVSS_4BYTE = %d\r\n", cur_sensing_value[1]);		
	}
	else
	{
		printf("IVDD_2BYTE = %d\r\n", adc_sensing_value[22]);
		printf("IVSS_2BYTE = %d\r\n", adc_sensing_value[32]);
	}
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

/*void resistance_measurement_1k(void)
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
		usleep(1);
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
}*/

void resistance_measurement_1(void)
{
	int i = 0;
	int j = 0;
	int ready = 0;
	char l_r=0;
	unsigned int adc_read_data_0 = 0;
	unsigned int adc_read_data_1 = 0;
	double real_read_data_0 = 0;
	double real_read_data_1 = 0;
	double adc_data_0_a = 0;	
	double adc_data_1_r = 0;								
	unsigned short crc_0 = 0;	//231015 Modify	
	unsigned short crc_1 = 0;	//231015 Modify		

	int res_adc_start = 0;	//231015 Modify
	int res_adc_adc_auto_end = 0;	//231015 Modify
	int res_adc_adc_auto_time = ADC_TIMEOUT;	//231015 Modify

	gpio->GPIO_DATA |= 0xaa;	//1ohm Resistance Sel	//231027 Modify 
	//gpio->GPIO_DATA = 0x1aa;	//1ohm Resistance Sel
	usleep(1000);
	ads124s08_rm->ADC_AUTO_DELAY = 2000;

	if(ads124s08_rm->ADC_STATUS & (ADC_STATUS_AUTO_READ_DONE)) ads124s08_rm->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE | ADC_Control_AUTO_READ_CLEAR;
	else ads124s08_rm->ADC_CONTROL = ADC_Control_AUTO_READ_ENABLE;

	res_adc_start = timeout_msclock();	//231015 Modify
	while(1)
	{
		if((ads124s08_rm->ADC_STATUS & (1<<2)))
		{
			ready = 1;
			break;
		}				
		//usleep(1000);			//231013 Modify	
		//j++;
		//if(j > 5) return 0;
		//if(j > 5000000) break;	//231013 Modify	
		usleep(1);				//231013 Modify	
		res_adc_adc_auto_end = timeout_msclock() - res_adc_start;	//231015 Modify
		if(res_adc_adc_auto_end >= res_adc_adc_auto_time) break;	//231015 Modify					
	}
	usleep(1000);
	if(ready)	
	{
		for(int i = 0; i<2 ; i++)
		{
			usleep(1000);
			if(i==0)	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x01;	
			else	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x03;	
			usleep(1000);		
			adc_read_data_0 = ads124s08_rm->ADC_AUTO_DATA;						
			crc_0 = ADC_CRC8((unsigned char *)&adc_read_data_0, sizeof(adc_read_data_0));	
			if(!crc_0)
			{
				if(adc_read_data_0 & 0x80000000)real_read_data_0 = ((float)((((~adc_read_data_0) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT;
				else real_read_data_0 = ((float)((adc_read_data_0 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT;						
				
				adc_read_data_0 = (adc_read_data_0 >>8);
				//printf("crc_rm_ok = %x / %f\r\n", adc_read_data_0, real_read_data_0);
			}
			else	
			{
				error_data.resistance_adc = ERROR_NG;	//231013 Modify
				//ads124s08_ads124s08_rm_init();			//231013 Modify	//231117 Modify									
				printf("Resistance_measurement_CRC[0]_NG\r\n");	
				ads124s08_ads124s08_rm_re_init();
			}

			usleep(1000);	//231013 Modify 
			if(i==0)	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x00;	
			else	ads124s08_rm->ADC_AUTO_DATA_SEL = 0x02;		
			usleep(1000);	//231013 Modify 
			adc_read_data_1 = ads124s08_rm->ADC_AUTO_DATA;
			crc_1 = ADC_CRC8((unsigned char *)&adc_read_data_1, sizeof(adc_read_data_1)); 
			if(!crc_1)
			{
				if(adc_read_data_1 & 0x80000000)real_read_data_1 = ((float)((((~adc_read_data_1) >> 8) & 0x007FFFFF)+1)) * (float)(-1.0) * (float)ADC_UNIT;
				else real_read_data_1 = ((float)((adc_read_data_1 >> 8) & 0x007FFFFF)) * (float)(1.0) * (float)ADC_UNIT;						
				
				adc_read_data_1 = (adc_read_data_1 >>8);
				//printf("crc_rm[0]_ok = %x / %f\r\n", adc_read_data_1, real_read_data_1);
			}
			else	
			{
				error_data.resistance_adc = ERROR_NG;		//231013 Modify
				//ads124s08_ads124s08_rm_init();				//231013 Modify	//231117 Modify										
				printf("Resistance_measurement_CRC[1]_NG\r\n");	
				ads124s08_ads124s08_rm_re_init();
			} 	

			if((crc_0 == 0) && (crc_1 == 0))	adc_data_0_a = real_read_data_0/1;	//231015 Modify	
			if((crc_0 == 0) && (crc_1 == 0))	adc_data_1_r = real_read_data_1 / adc_data_0_a;	//231015 Modify	
			
			if(i==0)	
			{
				if((crc_0 == 0) && (crc_1 == 0))	adc_res_value_r_1 = fabs((int)((adc_data_1_r-ADC_REG_R_OFFSET+register_offset.register_r)*10000));	//231015 Modify	
				//adc_res_value_r_1 = (int)((adc_data_1_r-ADC_REG_R_OFFSET+register_offset.register_r)*10000);
				//if(adc_res_value_r_1 > 3000000)	adc_res_value_r_1 = 0xffffffff;
				if(cprf) printf("1ohm_VR[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %d\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_res_value_r_1);	
			}
			else	
			{
				if((crc_0 == 0) && (crc_1 == 0))	adc_res_value_l_1 = fabs((int)((adc_data_1_r-ADC_REG_L_OFFSET+register_offset.register_l)*10000));	//231015 Modify	
				//adc_res_value_l_1 = (int)((adc_data_1_r-ADC_REG_L_OFFSET+register_offset.register_l)*10000);
				//if(adc_res_value_l_1 > 3000000)	adc_res_value_l_1 = 0xffffffff;
				if(cprf) printf("1ohm_VL[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %d\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_res_value_l_1);	
			}
			//printf("------------------------------------------------------\r\n");	
			//printf("V[0] = %f / R[0] = 1K / A[0] = %f / V[1] = %f / R[1] = %f\r\n", real_read_data_0, adc_data_0_a, real_read_data_1, adc_res_value_l_1);	
		}
		if((adc_res_value_r_1 == 0) && (adc_res_value_l_1 == 0))	
		{
			error_data.resistance_adc = ERROR_NG;
			printf("Resistance_measurement_Value_NG\r\n");
			ads124s08_ads124s08_rm_re_init();	
		}
	}
	else 	
	{
		error_data.resistance_adc = ERROR_NG;
		//ads124s08_ads124s08_rm_init();	//231117 Modify		
		printf("Resistance_measurement_Ready_NG\r\n");	
		ads124s08_ads124s08_rm_re_init();	
	}
}

void register_offset_load()
{
	FILE *reg_file;
	//char register_offset_str[64] = {0};
	memset(&register_offset, 0, sizeof(register_offset));
	usleep(1000);

	//sprintf(register_offset_str, "ls %s",REGISTER_OFFSET_FILE_PATH);
    //if(system("ls /f0/config/eregister_offset.info"))
	//if(system(register_offset_str))
	if(access(REGISTER_OFFSET_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       reg_file = fopen(REGISTER_OFFSET_FILE_PATH,"wb");                           
    }
    else
    {
        reg_file = fopen(REGISTER_OFFSET_FILE_PATH, "r");
        if(reg_file != NULL)	
		{
		fread(&register_offset, sizeof(register_offset), 1, reg_file);     
        usleep(1000);
		}
    }
	if(reg_file != NULL)	printf("register_offset.register_r = %f / register_offset.register_l = %f\r\n", register_offset.register_r, register_offset.register_l);
	if(reg_file != NULL)	fclose(reg_file);	
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
	memset(&elvss_cur_offset_cal, 0, sizeof(elvss_cur_offset_cal)); 
	memset(&avdd_cur_offset_cal, 0, sizeof(avdd_cur_offset_cal)); 
	memset(&vol_offset, 0, sizeof(vol_offset)); 				//231206 Modify
	//memset(&n_avdd_cur_cal, 0, sizeof(n_avdd_cur_cal));   
	//memset(&n_elvss_cur_cal, 0, sizeof(n_elvss_cur_cal)); 	
    usleep(1000);   

    //if(system("ls /f0/config/ads124_cal0.info"))
	if(access(ADS124_0_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ADS124_0_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_0_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&ads124_cal0, sizeof(ads124_cal0), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/ads124_cal1.info"))
	if(access(ADS124_1_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ADS124_1_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_1_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&ads124_cal1, sizeof(ads124_cal1), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/ads124_cal2.info"))
	if(access(ADS124_2_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ADS124_2_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_2_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&ads124_cal2, sizeof(ads124_cal2), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/ads124_cal3.info"))
	if(access(ADS124_3_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ADS124_3_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_3_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&ads124_cal3, sizeof(ads124_cal3), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/ads124_cal4.info"))
	if(access(ADS124_4_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ADS124_4_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADS124_4_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&ads124_cal4, sizeof(ads124_cal4), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/avdd_cur_cal.info"))
	if(access(AVDD_CUR_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(AVDD_CUR_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(AVDD_CUR_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&avdd_cur_cal, sizeof(avdd_cur_cal), 1, adc_file);  
        usleep(1000);
    }	
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/elvss_cur_cal.info"))
	if(access(ELVSS_CUR_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ELVSS_CUR_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ELVSS_CUR_CAL_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&elvss_cur_cal, sizeof(elvss_cur_cal), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);

    //if(system("ls /f0/config/elvss_offset_cal.info")) 
	if(access(ELVSS_CUR_OFFSET_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(ELVSS_CUR_OFFSET_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ELVSS_CUR_OFFSET_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&elvss_cur_offset_cal, sizeof(elvss_cur_offset_cal), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);	

    //if(system("ls /f0/config/avdd_offset_cal.info")) 
	if(access(AVDD_CUR_OFFSET_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       adc_file = fopen(AVDD_CUR_OFFSET_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(AVDD_CUR_OFFSET_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&avdd_cur_offset_cal, sizeof(avdd_cur_offset_cal), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);	

	if(access(ADC_OFFSET_FILE_PATH,F_OK) != 0)	//231206 Modify
    {
       adc_file = fopen(ADC_OFFSET_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(ADC_OFFSET_FILE_PATH, "r");
        if(adc_file != NULL)	fread(&vol_offset, sizeof(vol_offset), 1, adc_file);  
        usleep(1000);
    }
	if(adc_file != NULL)	fclose(adc_file);		

    /*if(system("ls /f0/config/n_elvss_cur_cal.info"))
    {
       fopen(N_ELVSS_CUR_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        adc_file = fopen(N_ELVSS_CUR_CAL_FILE_PATH, "r");
        fread(&n_elvss_cur_cal, sizeof(n_elvss_cur_cal), 1, adc_file);  
        usleep(1000);
    }*/	

	if(nprf)
	{
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
		printf("avdd_cur_cal.n_cur_1a_user_offset = %f\r\n", avdd_cur_cal.n_cur_1a_user_offset);

		/*printf("n_avdd_cur_cal.cur_100ma_value = %f / n_avdd_cur_cal.cur_0ma_value = %f\r\n", n_avdd_cur_cal.cur_100ma_value, n_avdd_cur_cal.cur_0ma_value);
		printf("n_avdd_cur_cal.cur_100ma_step = %f / n_avdd_cur_cal.cur_0ma_step = %f\r\n", n_avdd_cur_cal.cur_100ma_step, n_avdd_cur_cal.cur_0ma_step);
		printf("n_avdd_cur_cal.cur_100ma_ratio = %f / n_avdd_cur_cal.cur_100ma_offset = %f\r\n", n_avdd_cur_cal.cur_100ma_ratio, n_avdd_cur_cal.cur_100ma_offset);		
		printf("n_avdd_cur_cal.cur_100ma_user_offset = %f\r\n", n_avdd_cur_cal.cur_100ma_user_offset);

		printf("n_avdd_cur_cal.cur_1a_value = %f / n_avdd_cur_cal.cur_0ma_value = %f\r\n", n_avdd_cur_cal.cur_1a_value, n_avdd_cur_cal.cur_0a_value);
		printf("n_avdd_cur_cal.cur_1a_step = %f / n_avdd_cur_cal.cur_0ma_step = %f\r\n", n_avdd_cur_cal.cur_1a_step, n_avdd_cur_cal.cur_0a_step);
		printf("n_avdd_cur_cal.cur_1a_ratio = %f / n_avdd_cur_cal.cur_1a_offset = %f\r\n", n_avdd_cur_cal.cur_1a_ratio, n_avdd_cur_cal.cur_1a_offset);		
		printf("n_avdd_cur_cal.cur_1a_user_offset = %f\r\n", n_avdd_cur_cal.cur_1a_user_offset);	
		printf("nn_avdd_cur_cal.cur_1a_user_offset = %f\r\n", n_avdd_cur_cal.n_cur_1a_user_offset);*/	

		printf("elvss_cur_cal.cur_100ma_value = %f / elvss_cur_cal.cur_0ma_value = %f\r\n", elvss_cur_cal.cur_100ma_value, elvss_cur_cal.cur_0ma_value);
		printf("elvss_cur_cal.cur_100ma_step = %f / elvss_cur_cal.cur_0ma_step = %f\r\n", elvss_cur_cal.cur_100ma_step, elvss_cur_cal.cur_0ma_step);
		printf("elvss_cur_cal.cur_100ma_ratio = %f / elvss_cur_cal.cur_100ma_offset = %f\r\n", elvss_cur_cal.cur_100ma_ratio, elvss_cur_cal.cur_100ma_offset);		
		printf("elvss_cur_cal.cur_100ma_user_offset = %f\r\n", elvss_cur_cal.cur_100ma_user_offset);

		printf("elvss_cur_cal.cur_1a_value = %f / elvss_cur_cal.cur_0a_value = %f\r\n", elvss_cur_cal.cur_1a_value, elvss_cur_cal.cur_0a_value);
		printf("elvss_cur_cal.cur_1a_step = %f / elvss_cur_cal.cur_0a_step = %f\r\n", elvss_cur_cal.cur_1a_step, elvss_cur_cal.cur_0a_step);
		printf("elvss_cur_cal.cur_1a_ratio = %f / elvss_cur_cal.cur_1a_offset = %f\r\n", elvss_cur_cal.cur_1a_ratio, elvss_cur_cal.cur_1a_offset);		
		printf("elvss_cur_cal.cur_1a_user_offset = %f\r\n", elvss_cur_cal.cur_1a_user_offset);	
		printf("elvss_cur_cal.n_cur_1a_user_offset = %f\r\n", elvss_cur_cal.n_cur_1a_user_offset);	

		for(i = 0 ; i < 25 ; i++)	printf("elvss_cur_offset_cal.p_offset[%d] = %f / elvss_cur_offset_cal.n_offset[%d] = %f\r\n", i,elvss_cur_offset_cal.p_offset[i], i, elvss_cur_offset_cal.n_offset[i]);	
		for(i = 0 ; i < 25 ; i++)	printf("avdd_cur_offset_cal.p_offset[%d] = %f / avdd_cur_offset_cal.n_offset[%d] = %f\r\n", i,avdd_cur_offset_cal.p_offset[i], i, avdd_cur_offset_cal.n_offset[i]);	

		/*printf("n_elvss_cur_cal.cur_100ma_value = %f / n_elvss_cur_cal.cur_0ma_value = %f\r\n", n_elvss_cur_cal.cur_100ma_value, n_elvss_cur_cal.cur_0ma_value);
		printf("n_elvss_cur_cal.cur_100ma_step = %f / n_elvss_cur_cal.cur_0ma_step = %f\r\n", n_elvss_cur_cal.cur_100ma_step, n_elvss_cur_cal.cur_0ma_step);
		printf("n_elvss_cur_cal.cur_100ma_ratio = %f / n_elvss_cur_cal.cur_100ma_offset = %f\r\n", n_elvss_cur_cal.cur_100ma_ratio, n_elvss_cur_cal.cur_100ma_offset);		
		printf("n_elvss_cur_cal.cur_100ma_user_offset = %f\r\n", n_elvss_cur_cal.cur_100ma_user_offset);

		printf("n_elvss_cur_cal.cur_1a_value = %f / n_elvss_cur_cal.cur_0a_value = %f\r\n", n_elvss_cur_cal.cur_1a_value, n_elvss_cur_cal.cur_0a_value);
		printf("n_elvss_cur_cal.cur_1a_step = %f / n_elvss_cur_cal.cur_0a_step = %f\r\n", n_elvss_cur_cal.cur_1a_step, n_elvss_cur_cal.cur_0a_step);
		printf("n_elvss_cur_cal.cur_1a_ratio = %f / n_elvss_cur_cal.cur_1a_offset = %f\r\n", n_elvss_cur_cal.cur_1a_ratio, n_elvss_cur_cal.cur_1a_offset);		
		printf("n_elvss_cur_cal.cur_1a_user_offset = %f\r\n", n_elvss_cur_cal.cur_1a_user_offset);	
		printf("nn_elvss_cur_cal.cur_1a_user_offset = %f\r\n", n_elvss_cur_cal.n_cur_1a_user_offset);*/	
	}						
}

char adc_sen_monitoring(char *cmd, short data)
{
	int i = 0;
	int j = 0;
	int sesing_code = -1;
	char done = 0;
    /*const char *sensing_list[MAX_SENSING_COUNT] = {"DP0","AP0","LM6","PM8","PM0","DP1","AP1","LM7","PM9","PM1","DP2","AP2","I2C","LM0","PM2",
    "DP3", "AP3", "LOGIC", "LM1", "PM3", "ELVDD", "AP4", "IDD0", "LM2", "PM4", "ELVSS", "AP5", "IDD1", "LM3", "PM5", "NULL0","AP6","ISS0", 
	"LM4", "PM6", "NULL1","AP7", "ISS1", "LM5", "PM7"};*/	
    const char *sensing_list[MAX_SENSING_COUNT] = {"VDD11","ELVDD","LM_SPARE1","VOTP50","LDO_ELVDD","VDD18","VDD8_S","LM_SPARE2","PM_SPARE1","LDO_OSC","DP_SPARE1","VDD8_G","I2C","MON1","LDO_VGH",
    "DP_SPARE2", "VGH", "LOGIC", "MON2", "LDO_VGL", "AVDD", "VGL", "IDD0", "MON3", "LDO_VINT", "ELVSS", "VINT", "IDD1", "MON4", "VCIR", "NULL0","AP_SPARE1","ISS0", 
	"MON5", "VREF1", "NULL1","AP_SPARE2", "ISS1", "MON6", "VREG1"};		
	
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
		//ADC_AUTO_DATA_READ();
		ADC_SELECT_DATA_READ_AVG(sesing_code);
		if((adc_sensing_value[sesing_code])> data)
		{
			done = 1;
			break;
		}				
		//usleep(1000);
		j++;
		//if(j > 5) return 0;
		if(j > 50000) break;
	}

	if(done)	return 1;
	else 	
	{
		printf("Monitoring_Sensing_Fail Sensing_data = CH: %s / DATA: %d / Criteria = %d\r\n", cmd, adc_sensing_value[sesing_code], data);
		return 0;
	}
}

void ads124s08_ads124s08_rm_re_init(void)
{
    int i = 0;
	int j = 0;
	char ready = 0;
	unsigned char write_data;    
	int rm_init_start = 0;	//231015 Modify
	int rm_init_end = 0;	//231015 Modify
	int rm_init_time = ADC_TIMEOUT;	//231015 Modify
	int rm_init_error_time_delay= 5000;
	int cnt = 0;
	unsigned int init_data = 0;
    ads124s08_rm->ADC_CONTROL = ADC_Control_RESET;
	rm_init_start = timeout_msclock();	//231015 Modify
	while(1)
	{
		if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_RESET))
		{
			ready = 1;
			break;
		}				
		usleep(1);	//231013 Modify
		rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
		if(rm_init_end >= rm_init_time) break;	//231015 Modify			
	}
	usleep(50000);
	if(!ready)	
	{
		printf("----------ADS124_RESET_FAIL----------\r\n");
		return;			
	}

    printf("----------ads124s08_rm_re_init----------\r\n");	  
	for(i = 0 ; i < 18 ; i++)
    {
        if(ADC_RM_REG_SETTING_VALUE_LOAD((ADC_REG_ADDRESS_NUM)i, &write_data))
		{	
            ads124s08_rm->ADC_REG_ADDRESS = i;
            ads124s08_rm->ADC_REG_WRITE = write_data;
            //ads124s08_rm->ADC_CONTROL = ADC_Control_START_PIN_HIGH;
            ads124s08_rm->ADC_CONTROL = ADC_Control_REG_WRITE; 
			j=0;
			ready = 0;
			rm_init_start = 0;	//231015 Modify
			rm_init_end = 0;	//231015 Modify
			rm_init_start = timeout_msclock();	//231015 Modify				
			while(1)
			{
				if(!(ads124s08_rm->ADC_CONTROL&ADC_Control_REG_WRITE))
				{
					ready = 1;
					break;
				}				
				usleep(1);	//231015 Modify
				rm_init_end = timeout_msclock() - rm_init_start;	//231015 Modify
				if(rm_init_end >= rm_init_time) break;	//231015 Modify						
			}
			if(!ready)	
			{					
				printf("----------ADS124_RM_WRITE_FAIL----------\r\n");		
				return;										
			}							          
        } 
		usleep(10000);	
    }	  
}