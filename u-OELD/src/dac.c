#include "../include/dac.h"
#include "../include/fpga_reg.h"
#include <stdio.h>

extern int cprf;
struct SpiDevice dev_0;
struct SpiDevice dev_1;
struct SpiDevice dev_2;	

static BOOL DAC_REG_SETTING_VALUE_LOAD(DAC_REG_ADDRESS address, unsigned short *init_value);
/*static inline int msclock(void);
static inline int msclock(void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}*/

/*int msclock(void)
{
	struct timeval tv;
	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}*/
static BOOL DAC_REG_SETTING_VALUE_LOAD(DAC_REG_ADDRESS address, unsigned short *init_value)
{
	switch(address)
	{
 		case NOP:									//0x00
			return FALSE;
		case DEVICEID:								//0x01
			return FALSE;
		case DAC_STATUS:							//0x02
			return FALSE;
		case SPICONFIG:								//0x03
			*init_value = 0x0084;
			return TRUE;
		case GENCONFIG:								//0x04
			*init_value = 0x3F00;
			return TRUE;
		case BRDCONFIG:								//0x05
			*init_value = 0xF00F;
			return TRUE;			
		case SYNCCONFIG:							//0x06
			//*init_value = 0x0FF0;
			*init_value = 0x0000;
			return TRUE;				
		case TOGGCONFIG0:							//0x07
			return FALSE;
		case TOGGCONFIG1:							//0x08
			return FALSE;			
		case DACPWDWN:								//0x09
			*init_value = 0xF00F;
			return TRUE;		
		case NC:
			return FALSE;			
		case DACRANGE0:								//0x0B
			//*init_value = 0xCCCC;
			*init_value = 0x9999;
			return TRUE;			
		case DACRANGE1:								//0x0C
			//*init_value = 0xCCCC;
			*init_value = 0x9999;
			return TRUE;		
		case TRIGGER:								//0x0E
			return FALSE;		
		case BRDCAST:								//0x0F
			return FALSE;			
		default:
			return FALSE; 
	}
}

void dac_init(void)
{
	static unsigned short write_data;
	int j;
	int dac_i;
	char reg_data[3] = {0,};
	int rc;

	dev_0.filename = "/dev/spidev1.0";
	dev_0.mode = SPI_MODE_1;
	dev_0.bpw = 8;
	dev_0.speed = 50000000;
	
	dev_1.filename = "/dev/spidev1.1";
	dev_1.mode = SPI_MODE_1;
	dev_1.bpw = 8;
	dev_1.speed = 50000000;

	dev_2.filename = "/dev/spidev1.2";
	dev_2.mode = SPI_MODE_1;
	dev_2.bpw = 8;
	dev_2.speed = 50000000;
	/*
	 * Start the I2C device.
	 */
	rc = spi_start(&dev_0);
	if (rc) {
		printf("failed to start SPI_0 device\r\n");
		//return rc;
	}
	rc = spi_start(&dev_1);
	if (rc) {
		printf("failed to start SPI_1 device\r\n");
		//return rc;
	}
	rc = spi_start(&dev_2);	
	if (rc) {
		printf("failed to start SPI_2 device\r\n");
		//return rc;
	}	


	for(j=0x00; j<0x10; j++)	
	{
		if(DAC_REG_SETTING_VALUE_LOAD((DAC_REG_ADDRESS)j, &write_data))
		{
			reg_data[2] = write_data & 0xff;
			reg_data[1] = (write_data>>8) & 0xff;
			reg_data[0] = j;
			//printf("j = %d / write_data = %x\r\n", j, write_data);
			transfer_spi_dac(dev_0.fd,reg_data,3);
			transfer_spi_dac(dev_1.fd,reg_data,3);
			transfer_spi_dac(dev_2.fd,reg_data,3);
		}
	} 

	pattern_generator->PG_CONTROL = DAC_nCLR_LOW;
	usleep(10);
	pattern_generator->PG_CONTROL = DAC_nCLR_HIGH;	
	usleep(1000);	
	pattern_generator->PG_CONTROL = DAC_nCLR_LOW;

	pattern_generator->PG_CONTROL = OUT_EN_ON;

	dac_cal_load();	

	signal_group.signal_config->dc_voltage[LOGIC] = 1800;
	dac_set_for_logic();

	signal_group.signal_config->dc_voltage[I2C] = 1800;
	dac_set_for_i2c();	
}

void Power_Supply_Voltage_load(void)
{
    int i, j;
    short v1p = 0;

    //pvsig_onoff(false);
    //nvsig_onoff(false); 
    for(i = 0 ; i < 20 ; i++)
    {
        if(v1p < abs(signal_group.signal_config->dc_voltage[i])) v1p = abs(signal_group.signal_config->dc_voltage[i]);
    }

    if(v1p != 0)
    {
        if(v1p <= 5000) 
        {     
            pvsig_voltage_set(power_vol_10);
            nvsig_voltage_set(power_vol_10);  
            pvsig_onoff(true);
            nvsig_onoff(true);                   
        }
        else if(v1p <= 10000) 
        {    
            pvsig_voltage_set(power_vol_15);
            nvsig_voltage_set(power_vol_15);    
            pvsig_onoff(true);
            nvsig_onoff(true);                   
        }
        else if(v1p <= 15000) 
        {     
            pvsig_voltage_set(power_vol_20);
            nvsig_voltage_set(power_vol_20); 
            pvsig_onoff(true);
            nvsig_onoff(true);                     
        }
        else if(v1p <= 20000) 
        {   
            pvsig_voltage_set(power_vol_25);
            nvsig_voltage_set(power_vol_25);  
            pvsig_onoff(true);
            nvsig_onoff(true);                    
        }
        else if(v1p <= 25000) 
        {    
            pvsig_voltage_set(power_vol_28);
            nvsig_voltage_set(power_vol_28); 
            pvsig_onoff(true);
            nvsig_onoff(true);                     
        }
        else 
        {        
			pvsig_voltage_set(power_vol_min);
			nvsig_voltage_set(power_vol_min);  
			pvsig_onoff(true);
			nvsig_onoff(true);     
        }  
    }
	else 
	{
		pvsig_voltage_set(power_vol_min);
		nvsig_voltage_set(power_vol_min);  
		pvsig_onoff(true);
		nvsig_onoff(true);  		
	} 
}    

void Power_Supply_Voltage_load_pjh(short *vol_data)
{
    int i, j;
    short v1p = 0;

    for(i = 0 ; i < 20 ; i++)
    {
		if(v1p < abs(*(vol_data+i))) v1p = abs(*(vol_data+i));
    }
	printf("TP1 = %d\r\n", v1p);

    if(v1p != 0)
    {
        if(v1p <= 5000) 
        {     
            pvsig_voltage_set(power_vol_10);
            nvsig_voltage_set(power_vol_10);  
            pvsig_onoff(true);
            nvsig_onoff(true);                   
        }
        else if(v1p <= 10000) 
        {    
            pvsig_voltage_set(power_vol_15);
            nvsig_voltage_set(power_vol_15);    
            pvsig_onoff(true);
            nvsig_onoff(true);                   
        }
        else if(v1p <= 15000) 
        {     
            pvsig_voltage_set(power_vol_20);
            nvsig_voltage_set(power_vol_20); 
            pvsig_onoff(true);
            nvsig_onoff(true);                     
        }
        else if(v1p <= 20000) 
        {   
            pvsig_voltage_set(power_vol_25);
            nvsig_voltage_set(power_vol_25);  
            pvsig_onoff(true);
            nvsig_onoff(true);                    
        }
        else if(v1p <= 25000) 
        {    
            pvsig_voltage_set(power_vol_28);
            nvsig_voltage_set(power_vol_28); 
            pvsig_onoff(true);
            nvsig_onoff(true);                     
        }
        else 
        {        
			pvsig_voltage_set(power_vol_min);
			nvsig_voltage_set(power_vol_min);  
			pvsig_onoff(true);
			nvsig_onoff(true);     
        }  
    }
	else 
	{
		pvsig_voltage_set(power_vol_min);
		nvsig_voltage_set(power_vol_min);  
		pvsig_onoff(true);
		nvsig_onoff(true);  		
	}
} 

void dac_set(int dac, uint8_t *write_buf, uint32_t buf_len)
{
    if(dac == DAC0)  transfer_spi_dac(dev_0.fd,write_buf,buf_len);
    else if(dac == DAC1)    transfer_spi_dac(dev_1.fd,write_buf,buf_len);    
    else if(dac ==DAC2)    transfer_spi_dac(dev_2.fd,write_buf,buf_len); 
}

void dac_Manual_cal(int outch)
{
	char ch;
	short a = 0;
	short vol = 0;
	char c[3]  = {0,};
	int size = 0;
	int mode = 0;
	int i = 0;
	char dac_num;
	char dac_ch = 0;
	unsigned char tbreak = 0;	
	printf("ch[%d] CAL START\r\n", outch);

	if(outch < 2)	
	{
		dac_num = 0;
		dac_ch = outch;
	}	
	else if(outch <10)
	{	
		dac_num = 1;
		dac_ch = outch - 2;
	}
	else if(outch <14)	
	{
		dac_num =2;
		dac_ch = outch - 10;
	}
	else dac_num = 10;

	for(i = 0 ; i < 5 ; i++)
	{
		printf("mode = %d / dac_num = %d / outch = %d / dac_ch = %d\r\n", mode, dac_num, outch, dac_ch);
		tbreak = 0;
		if(mode==0)	vol = 25000;
		else if(mode==1) vol = 10000;
		else if(mode==2) vol = 0;		
		else if(mode==3) vol = -10000;
		else if(mode==4) vol = -25000;		

		if(mode==2)	signal_group.signal_config->dc_voltage[outch] = 1; 
		else signal_group.signal_config->dc_voltage[outch] = vol;	
		Power_Supply_Voltage_load();
		pattern_generator->PG_CONTROL = OUT_EN_ON;
		a = (short)(((65535/10)*(((vol/1000)/5.3)+5)));	
		
		c[2] = a &0xff;
		c[1] = (a>>8) &0xff;
		c[0] = 20+dac_ch;	
		dac_set(dac_num,c,3);	

		while(1) 
		{		
			ch = getch();
			
			if(ch == '=')
			{	
				a = a + 1;
				c[2] = a &0xff;
				c[1] = (a>>8) &0xff;
				c[0] = 20+dac_ch;	
				dac_set(dac_num,c,3);					
				printf("DAC Dmax offset = %x\r\n", a);
			}
			else if(ch == '-')
			{
				a = a - 1;
				c[2] = a &0xff;
				c[1] = (a>>8) &0xff;
				c[0] = 20+dac_ch;		
				dac_set(dac_num,c,3);				
				printf("DAC Dmax offset = %x\r\n", a);
			}
			else if(ch == '\r'|| ch == '\n') 
			{
				if(mode==0)
				{
					dac_cal[outch].dac_25v_step = a&0x000000000000ffff;
					dac_cal[outch].dac_25v_value = 25000;
					printf("dac_cal[%d].dac_25v_step = %f / dac_cal[%d].dac_25v_VALUE = %f\r\n", outch, dac_cal[outch].dac_25v_step, outch,dac_cal[outch].dac_25v_value);
					mode = 1;
				}
				else if(mode==1)
				{
					dac_cal[outch].dac_10v_step = a&0x000000000000ffff;;
					dac_cal[outch].dac_10v_value = 10000;
					printf("dac_cal[%d].dac_10v_step = %f / dac_cal[%d].dac_10v_VALUE = %f\r\n", outch, dac_cal[outch].dac_10v_step, outch, dac_cal[outch].dac_10v_value);
					mode = 2;
				}
				else if(mode==2)
				{
					//dac_cal[outch].dac_n0v_step = abs(a);
					dac_cal[outch].dac_n0v_step = a&0x000000000000ffff;;
					dac_cal[outch].dac_0v_step = a&0x000000000000ffff;;
					dac_cal[outch].dac_0v_value = 0;
					printf("dac_cal[%d].dac_0v_step = %f / dac_cal[%d].dac_0v_VALUE = %f\r\n", outch, dac_cal[outch].dac_0v_step, outch, dac_cal[outch].dac_0v_value);
					mode = 3;
				}
				else if(mode==3)
				{
					dac_cal[outch].dac_n10v_step = a&0x000000000000ffff;;
					dac_cal[outch].dac_n10v_value = -10000;
					printf("dac_cal[%d].dac_-10v_step = %f / dac_cal[%d].dac_-10v_VALUE = %f\r\n", outch, dac_cal[outch].dac_n10v_step, outch, dac_cal[outch].dac_n10v_value);
					mode = 4;
				}
				else if(mode==4)
				{
					dac_cal[outch].dac_n25v_step = a&0x000000000000ffff;;
					dac_cal[outch].dac_n25v_value = -25000;
					printf("dac_cal[%d].dac_n25v_step = %f / dac_cal[%d].dac_n25v_VALUE = %f\r\n", outch, dac_cal[outch].dac_n25v_step, outch, dac_cal[outch].dac_n25v_value);
					dac_power_off();
				}

							
				putchar(ch);
				tbreak = 1;
				break;
			}			
			else if((ch == 'q') || (ch=='Q')) 
			{
				tbreak = 1;
				break;
			}
			usleep(3000);
		}
	}
	if(mode==4)
	{
		dac_cal[outch].dac_0to10_ratio = (double)((dac_cal[outch].dac_10v_step - dac_cal[outch].dac_0v_step)) / (double)((dac_cal[outch].dac_10v_value - dac_cal[outch].dac_0v_value));
		dac_cal[outch].dac_10to25_ratio = (double)((dac_cal[outch].dac_25v_step - dac_cal[outch].dac_10v_step)) / (double)((dac_cal[outch].dac_25v_value - dac_cal[outch].dac_10v_value));
		dac_cal[outch].dac_0ton10_ratio = (double)((dac_cal[outch].dac_0v_step - dac_cal[outch].dac_n10v_step)) / (double)((dac_cal[outch].dac_0v_value - dac_cal[outch].dac_n10v_value));
		dac_cal[outch].dac_n10ton25_ratio =(double)((dac_cal[outch].dac_n10v_step - dac_cal[outch].dac_n25v_step)) / (double)((dac_cal[outch].dac_n10v_value - dac_cal[outch].dac_n25v_value));
		//dac_cal[outch].dac__0to10_offset = (dac_cal[outch].dac_0v_value+25000) - (dac_cal[outch].dac_0to10_ratio * dac_cal[outch].dac_0v_step);
		//dac_cal[outch].dac__10to25_offset = (dac_cal[outch].dac_10v_value+25000) - (dac_cal[outch].dac_10to25_ratio * dac_cal[outch].dac_10v_step);
		//dac_cal[outch].dac__0ton10_offset = (dac_cal[outch].dac_n10v_value+25000) - (dac_cal[outch].dac_0ton10_ratio * dac_cal[outch].dac_n10v_step);
		//dac_cal[outch].dac__n10to_n25_offset = (dac_cal[outch].dac_n25v_value+25000) - (dac_cal[outch].dac_n10ton25_ratio * dac_cal[outch].dac_n25v_step);			

		printf("dac_cal[%d].dac_0v_value = %f / dac_0v_step = %f\r\n", outch, dac_cal[outch].dac_0v_value, dac_cal[outch].dac_0v_step);
		printf("dac_cal[%d].dac_10v_value = %f / dac_10v_step = %f\r\n", outch, dac_cal[outch].dac_10v_value, dac_cal[outch].dac_10v_step);	
		printf("dac_cal[%d].dac_25v_value = %f / dac_25v_step = %f\r\n", outch, dac_cal[outch].dac_25v_value, dac_cal[outch].dac_25v_step);	
		printf("dac_cal[%d].dac_n10v_value = %f / dac_n10v_step = %f\r\n", outch, dac_cal[outch].dac_n10v_value, dac_cal[outch].dac_n10v_step);	
		printf("dac_cal[%d].dac_n25v_value = %f / dac_n25v_step = %f\r\n", outch, dac_cal[outch].dac_n25v_value, dac_cal[outch].dac_n25v_step);	
		printf("///////////////////////////////////////////////////////////////////////////////////////////\r\n");	
		
		printf("dac_cal[%d].dac_0to10_ratio = %f\r\n", outch, dac_cal[outch].dac_0to10_ratio);
		printf("dac_cal[%d].dac_10to25_ratio = %f\r\n", outch, dac_cal[outch].dac_10to25_ratio);
		printf("dac_cal[%d].dac_0ton10_ratio = %f\r\n", outch, dac_cal[outch].dac_0ton10_ratio);	
		printf("dac_cal[%d].dac_n10ton25_ratio = %f\r\n", outch, dac_cal[outch].dac_n10ton25_ratio);			
	}
}

void dac_MTP(int outch)
{
	char ch;
	short a = 0;
	short vol = 0;
	char c[3]  = {0,};
	int size = 0;
	int mode = 0;
	int i = 0;
	char dac_num;
	char dac_ch = 0;
	unsigned char tbreak = 0;	
	printf("ch[%d] MTP START\r\n", outch);

	if(outch < 2)	
	{
		dac_num = 0;
		dac_ch = outch;
	}	
	else if(outch <10)
	{	
		dac_num = 1;
		dac_ch = outch - 2;
	}
	else if(outch <14)	
	{
		dac_num =2;
		dac_ch = outch - 10;
	}
	else dac_num = 10;

	printf("mode = %d / dac_num = %d / outch = %d / dac_ch = %d / data = %d\r\n", mode, dac_num, outch, dac_ch, signal_group.signal_config->dc_voltage[outch]);
	tbreak = 0;
	Power_Supply_Voltage_load();
	pattern_generator->PG_CONTROL = OUT_EN_ON;
	a = dac_data_backup[outch];	
	
	c[2] = a &0xff;
	c[1] = (a>>8) &0xff;
	c[0] = 20+dac_ch;	
	dac_set(dac_num,c,3);	

	while(1) 
	{		
		ch = getch();
		
		if(ch == '=')
		{	
			a = a + 1;
			c[2] = a &0xff;
			c[1] = (a>>8) &0xff;
			c[0] = 20+dac_ch;	
			dac_set(dac_num,c,3);					
			printf("DAC Dmax offset = %x\r\n", a);
		}
		else if(ch == '-')
		{
			a = a - 1;
			c[2] = a &0xff;
			c[1] = (a>>8) &0xff;
			c[0] = 20+dac_ch;		
			dac_set(dac_num,c,3);				
			printf("DAC Dmax offsetr = %x\r\n",a);
		}
		else if(ch == '\r'|| ch == '\n') 
		{

			dac_data_backup[outch] = a;			
			putchar(ch);
			tbreak = 1;
			break;
		}			
		else if((ch == 'q') || (ch=='Q')) 
		{
			tbreak = 1;
			break;
		}
		usleep(3000);
	}
}

void SET_DAC_OUTPUT_VALUE(int outch)
{	
	char c[3]  = {0,};
	double dac_data = 0;
	unsigned char dac_num;
	unsigned char dac_ch = 0;

	if(outch < 2)
	{
		dac_num = 0;
		dac_ch = outch;
	}	
	else if(outch <10)
	{	
		dac_num = 1;
		dac_ch = outch - 2;
	}
	else if(outch <14)	
	{
		dac_num =2;
		dac_ch = outch - 10;
	}
	else dac_num = 10;

	if(signal_group.signal_config->dc_voltage[outch] > 25000)	signal_group.signal_config->dc_voltage[outch] = 25000;
	if(signal_group.signal_config->dc_voltage[outch] <-25000)	signal_group.signal_config->dc_voltage[outch] = -25000;

	if(signal_group.signal_config->dc_voltage[outch] >= 0)
	{
		if(signal_group.signal_config->dc_voltage[outch] <=10000)
		{
			//dac_data = signal_group.signal_config->dc_voltage[0] * dac_cal[0].dac_0to10_ratio + dac_cal[0].dac_0to10_offset;	
			dac_data = ((double)signal_group.signal_config->dc_voltage[outch] * dac_cal[outch].dac_0to10_ratio) +(double)dac_cal[outch].dac_0v_step+dac_cal[outch].dac_0to10_offset;
		}	
		else if(signal_group.signal_config->dc_voltage[outch] <=25000)
		{
			//dac_data = signal_group.signal_config->dc_voltage[0] * dac_cal[0].dac_10to25_ratio + dac_cal[0].dac_10to25_offset;
			dac_data = ((double)signal_group.signal_config->dc_voltage[outch] * dac_cal[outch].dac_10to25_ratio) +(double)dac_cal[outch].dac_0v_step+dac_cal[outch].dac_10to25_offset;			
		} 
	}
	else 
	{
		if(signal_group.signal_config->dc_voltage[outch] >= -10000)
		{
			dac_data = ((double)signal_group.signal_config->dc_voltage[outch] * dac_cal[outch].dac_0ton10_ratio) - (double)dac_cal[outch].dac_n0v_step+dac_cal[outch].dac_0ton10_offset;
		}	
		else if(signal_group.signal_config->dc_voltage[outch] >=-25000)
		{
			dac_data = ((double)signal_group.signal_config->dc_voltage[outch] * dac_cal[outch].dac_n10ton25_ratio) -(double)dac_cal[outch].dac_n0v_step+dac_cal[outch].dac_n10to_n25_offset;		
		}		
	}
	dac_data_backup[outch] = dac_data;
	//Power_Supply_Voltage_load();
	pattern_generator->PG_CONTROL = OUT_EN_ON;
	c[2] = (short)dac_data &0xff;
	c[1] = ((short)dac_data>>8) &0xff;
	c[0] = 20+dac_ch;	
	dac_set(dac_num,c,3);
	usleep(1000);					
	if(cprf) printf("DAC Dmax offset = %x / dac_num = %d / dac_ch = %d \r\n", (short)dac_data, dac_num, dac_ch);
}

void dac_cal_load()
{
	int i= 0;
    FILE *dac_file;
    memset(&dac_cal, 0, 64);
  
    usleep(1000);   

    if(system("ls /f0/config/dac_cal.info"))
    {
       fopen(DAC_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        dac_file = fopen(DAC_CAL_FILE_PATH, "r");
        fread(&dac_cal, sizeof(dac_cal), 1, dac_file);  
        usleep(1000);
    }

	for(i =0 ; i<15 ; i++)	
	{
		printf("dac_cal[%d].dac_0v_value = %f / dac_0v_step = %f\r\n", i, dac_cal[i].dac_0v_value, dac_cal[i].dac_0v_step);
		printf("dac_cal[%d].dac_10v_value = %f / dac_10v_step = %f\r\n", i, dac_cal[i].dac_10v_value, dac_cal[i].dac_10v_step);	
		printf("dac_cal[%d].dac_25v_value = %f / dac_25v_step = %f\r\n", i, dac_cal[i].dac_25v_value, dac_cal[i].dac_25v_step);	
		printf("dac_cal[%d].dac_n10v_value = %f / dac_n10v_step = %f\r\n", i, dac_cal[i].dac_n10v_value, dac_cal[i].dac_n10v_step);	
		printf("dac_cal[%d].dac_n25v_value = %f / dac_n25v_step = %f\r\n", i, dac_cal[i].dac_n25v_value, dac_cal[i].dac_n25v_step);			
		printf("dac_cal[%d].dac_0to10_ratio = %f / dac__0to10v_offset = %f\r\n", i, dac_cal[i].dac_0to10_ratio, dac_cal[i].dac_0to10_offset);
		printf("dac_cal[%d].dac_10to25_ratio = %f / dac__10vto25voffset = %f\r\n", i, dac_cal[i].dac_10to25_ratio, dac_cal[i].dac_10to25_offset);
		printf("dac_cal[%d].dac_0ton10_ratio = %f / dac__0ton10_offset = %f\r\n", i, dac_cal[i].dac_0ton10_ratio, dac_cal[i].dac_0ton10_offset);	
		printf("dac_cal[%d].dac_n10ton25_ratio = %f / dac__n10to_n25_offset = %f\r\n", i, dac_cal[i].dac_n10ton25_ratio, dac_cal[i].dac_n10to_n25_offset);		
	}		
}

void dac_set_for_logic(void)
{
	short a = 0;
	char c[3]  = {0,};
	float vol = 0;
	int t, dt;
	unsigned char i=0,*id;

	vol = signal_group.signal_config->dc_voltage[LOGIC];									
	if(vol > 5000)	vol =5000;	
	a = (short)(((65535/10)*(((vol/1000))+4.85)));	
	if(cprf) printf("logic_value = %x\r\n",a);	
	//Power_Supply_Voltage_load();
	c[2] = a &0xff;
	c[1] = (a>>8) &0xff;
	c[0] = 25;	
	dac_set(DAC0,c,3);		
}

void dac_set_for_i2c(void)
{
	short a = 0;
	char c[3]  = {0,};
	float vol = 0;
	int t, dt;
	unsigned char i=0,*id;

	vol = signal_group.signal_config->dc_voltage[I2C];	
	if(vol > 5000)	vol =5000;								

	a = (short)(((65535/10)*(((vol/1000))+4.85)));	
	if(cprf) printf("i2c_value = %x\r\n",a);	
	//Power_Supply_Voltage_load();
	c[2] = a &0xff;
	c[1] = (a>>8) &0xff;
	c[0] = 24;	
	dac_set(DAC0,c,3);		
}