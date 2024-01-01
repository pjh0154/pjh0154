#include "../include/dac.h"
#include "../include/fpga_reg.h"
#include "../include/serial_dev_2.h"
#include "../include/gpio-dev.h"
#include <stdio.h>

extern int cprf;
struct SpiDevice dev_0;
struct SpiDevice dev_1;
struct SpiDevice dev_2;	
extern int kbhit(void);
extern unsigned char readch(void);

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

	signal_group.signal_config.dc_voltage[LOGIC] = 1800;
	dac_set_for_logic();

	signal_group.signal_config.dc_voltage[I2C] = 1800;
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
        if(v1p < abs(signal_group.signal_config.dc_voltage[i])) v1p = abs(signal_group.signal_config.dc_voltage[i]);
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

		if(mode==2)	signal_group.signal_config.dc_voltage[outch] = 1; 
		else signal_group.signal_config.dc_voltage[outch] = vol;	
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

	printf("mode = %d / dac_num = %d / outch = %d / dac_ch = %d / data = %d\r\n", mode, dac_num, outch, dac_ch, signal_group.signal_config.dc_voltage[outch]);
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
	char c[4]  = {0,};	
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

	if(signal_group.signal_config.dc_voltage[outch] > 25000)	signal_group.signal_config.dc_voltage[outch] = 25000;
	if(signal_group.signal_config.dc_voltage[outch] <-25000)	signal_group.signal_config.dc_voltage[outch] = -25000;

	if(signal_group.signal_config.dc_voltage[outch] >= 0)
	{
		if(signal_group.signal_config.dc_voltage[outch] <=10000)
		{
			//dac_data = signal_group.signal_config.dc_voltage[0] * dac_cal[0].dac_0to10_ratio + dac_cal[0].dac_0to10_offset;	
			dac_data = ((double)signal_group.signal_config.dc_voltage[outch] * dac_cal[outch].dac_0to10_ratio) +(double)dac_cal[outch].dac_0v_step+dac_cal[outch].dac_0to10_offset;
		}	
		else if(signal_group.signal_config.dc_voltage[outch] <=25000)
		{
			//dac_data = signal_group.signal_config.dc_voltage[0] * dac_cal[0].dac_10to25_ratio + dac_cal[0].dac_10to25_offset;
			dac_data = ((double)signal_group.signal_config.dc_voltage[outch] * dac_cal[outch].dac_10to25_ratio) +(double)dac_cal[outch].dac_0v_step+dac_cal[outch].dac_10to25_offset;			
		} 
	}
	else 
	{
		if(signal_group.signal_config.dc_voltage[outch] >= -10000)
		{
			dac_data = ((double)signal_group.signal_config.dc_voltage[outch] * dac_cal[outch].dac_0ton10_ratio) - (double)dac_cal[outch].dac_n0v_step+dac_cal[outch].dac_0ton10_offset;
		}	
		else if(signal_group.signal_config.dc_voltage[outch] >=-25000)
		{
			dac_data = ((double)signal_group.signal_config.dc_voltage[outch] * dac_cal[outch].dac_n10ton25_ratio) -(double)dac_cal[outch].dac_n0v_step+dac_cal[outch].dac_n10to_n25_offset;		
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

    //if(system("ls /f0/config/dac_cal.info"))
	if(access(DAC_CAL_FILE_PATH,F_OK) != 0)	//230929 Modify
    {
       dac_file = fopen(DAC_CAL_FILE_PATH,"wb");                             
    }
    else
    {
        dac_file = fopen(DAC_CAL_FILE_PATH, "r");
        if(dac_file != NULL)	fread(&dac_cal, sizeof(dac_cal), 1, dac_file);  
        usleep(1000);
    }

	if(dac_file != NULL)
	{
		for(i =0 ; i<14 ; i++)	
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
		fclose(dac_file);
	}		
}

void dac_set_for_logic(void)
{
	short a = 0;
	char c[4]  = {0,};
	float vol = 0;
	int t, dt;
	unsigned char i=0,*id;

	vol = signal_group.signal_config.dc_voltage[LOGIC];									
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
	char c[4]  = {0,};
	float vol = 0;
	int t, dt;
	unsigned char i=0,*id;

	vol = signal_group.signal_config.dc_voltage[I2C];	
	if(vol > 5000)	vol =5000;								

	a = (short)(((65535/10)*(((vol/1000))+4.85)));	
	if(cprf) printf("i2c_value = %x\r\n",a);	
	//Power_Supply_Voltage_load();
	c[2] = a &0xff;
	c[1] = (a>>8) &0xff;
	c[0] = 24;	
	dac_set(DAC0,c,3);		
}

//void dac_auto_cal(int outch)
unsigned char dac_auto_cal(int outch)
{
	unsigned char category_start_flag = 0xff;
	char ch = 0;
	short a = 0;
	short vol = 0;
	char c[3]  = {0,};
	int size = 0;
	int mode = 0;
	int i = 0;
	int auto_cal_data_divide_1000 = 0;
	int count = 0;
	char dac_num;
	char dac_ch = 0;
	unsigned char autocal_result = 0;
	unsigned char tbreak = 0;	
	unsigned char data[100];
	unsigned char vol_cur_select_task_result = 0; 

    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    if(!vol_cur_select_task_result)
	{	
		printf("ch[%d] CAL START\r\n", outch);
		memset(&data,0, sizeof(data));
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

		//for(i = 0 ; i < 1 ; i++)
		for(i = 0 ; i < DAC_CAL_N25V+1 ; i++)
		{
			printf("mode = %d / dac_num = %d / outch = %d / dac_ch = %d\r\n", mode, dac_num, outch, dac_ch);
			count = 0;
			tbreak = 0;
			if(mode==DAC_CAL_25V)	vol = 25000;
			else if(mode==DAC_CAL_10V) vol = 10000;
			else if(mode==DAC_CAL_0V) vol = 0;		
			else if(mode==DAC_CAL_N10V) vol = -10000;
			else if(mode==DAC_CAL_N25V) vol = -25000;		

			if(mode==2)	signal_group.signal_config.dc_voltage[outch] = 1; 
			else signal_group.signal_config.dc_voltage[outch] = vol;	
			Power_Supply_Voltage_load();
            if(dac_cal[outch].dac_25v_step == 0)
            {			
				pattern_generator->PG_CONTROL = OUT_EN_ON;
				a = (short)(((65535/10)*(((vol/1000)/5.3)+5)));			
				c[2] = a &0xff;
				c[1] = (a>>8) &0xff;
				c[0] = 20+dac_ch;	
				dac_set(dac_num,c,3);	
			}
			else	
			{
				SET_DAC_OUTPUT_VALUE(outch);   
				if(mode==DAC_CAL_25V)	a = dac_cal[outch].dac_25v_step;
				else if(mode==DAC_CAL_10V) a = dac_cal[outch].dac_10v_step;
				else if(mode==DAC_CAL_0V) a = dac_cal[outch].dac_0v_step;		
				else if(mode==DAC_CAL_N10V) a = dac_cal[outch].dac_n10v_step;
				else if(mode==DAC_CAL_N25V) a = dac_cal[outch].dac_n25v_step;						
			} 
			gpio_enble->GPIO_DATA |= (0x00000001<<(outch&0x3fff));
			if(outch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
			else if(outch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
			else if(outch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
			else if(outch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
			else if(outch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
			else if(outch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
			//data[0] = TASK_FOR_VOL_CHECK;	
			data[0] = AUTO_CAL_TASK;	
			data[1] = outch;	
			if(mode==DAC_CAL_25V)	printf("[DAC_25V_CAL_START]\r\n");
			else if(mode==DAC_CAL_10V) printf("[DAC_10V_CAL_START]\r\n");
			else if(mode==DAC_CAL_0V) printf("[DAC_0V_CAL_START]\r\n");		
			else if(mode==DAC_CAL_N10V) printf("[DAC_N10V_CAL_START]\r\n");
			else if(mode==DAC_CAL_N25V) printf("[DAC_N25V_CAL_START]\r\n");	
			while(1) 
			{
				ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
				if(category_start_flag == DAC_ADC_AUTO_STOP)	
				{
					autocal_result = DAC_CAL_CANCEL;
					return autocal_result;
				}	
				/*if(kbhit())	ch = readch();
				usleep(100000);
				if((ch == 'q') || (ch=='Q'))	
				{
					autocal_result = DAC_AUTOCHECK_CANCEL;
					return	autocal_result;			
				}*/

				ex_port_send_task(data,AUTO_CAL_TASK_LEN);		
				ex_port_serial_task(); 
				//ex_recive_data_delay_task(1000);
				usleep(200000);
				auto_cal_data_divide_1000 = auto_cal_data/1000; 
				if(mode == DAC_CAL_25V)
				{					
					if((auto_cal_data_divide_1000 < 0) || (auto_cal_data_divide_1000 < 24000) || (auto_cal_data_divide_1000 > 27000))
					{
						autocal_result = outch+1;
						return	autocal_result;								
					}
					if((auto_cal_data_divide_1000 <= 25001) && (auto_cal_data_divide_1000 >= 24999))
					{
						dac_cal[outch].dac_25v_step = a&0x000000000000ffff;
						dac_cal[outch].dac_25v_value = auto_cal_data_divide_1000;
						mode = DAC_CAL_10V;
						printf("dac_cal[%d].dac_25v_step = %f / dac_cal[%d].dac_25v_VALUE = %f\r\n", outch, dac_cal[outch].dac_25v_step, outch,dac_cal[outch].dac_25v_value);			
						break;
					}
					else if(auto_cal_data_divide_1000 < 25000)
					{
						if(auto_cal_data_divide_1000 < 24990)	
						{
							a = a + 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[+10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}					
						else if(auto_cal_data_divide_1000 < 24999)	
						{
							a = a + 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[+1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					else if(auto_cal_data_divide_1000 > 25000)
					{
						if(auto_cal_data_divide_1000 > 25010)
						{
							a = a - 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[-10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);							
						}
						else if(auto_cal_data_divide_1000 > 25001)
						{
							a = a - 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[-1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					count++;
				}
				else if(mode == DAC_CAL_10V)
				{
					if((auto_cal_data_divide_1000 < 0) || (auto_cal_data_divide_1000 < 8000) || (auto_cal_data_divide_1000 > 12000))
					{
						autocal_result = outch+1;
						return	autocal_result;								
					}					
					if((auto_cal_data_divide_1000 <= 10001) && (auto_cal_data_divide_1000 >= 9999))
					{
						dac_cal[outch].dac_10v_step = a&0x000000000000ffff;
						dac_cal[outch].dac_10v_value = auto_cal_data_divide_1000;
						mode = DAC_CAL_0V;
						printf("dac_cal[%d].dac_10v_step = %f / dac_cal[%d].dac_10v_VALUE = %f\r\n", outch, dac_cal[outch].dac_10v_step, outch,dac_cal[outch].dac_10v_value);			
						break;
					}
					else if(auto_cal_data_divide_1000 < 10000)
					{
						if(auto_cal_data_divide_1000 < 9990)	
						{
							a = a + 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[+10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}					
						else if(auto_cal_data_divide_1000 < 9999)	
						{
							a = a + 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[+1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					else if(auto_cal_data_divide_1000 > 10000)
					{
						if(auto_cal_data_divide_1000 > 10010)
						{
							a = a - 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[-10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);							
						}
						else if(auto_cal_data_divide_1000 > 10001)
						{
							a = a - 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[-1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					count++;
				}	
				else if(mode == DAC_CAL_0V)
				{					
					if((auto_cal_data_divide_1000 < -2000) || (auto_cal_data_divide_1000 > 2000))
					{
						autocal_result = outch+1;
						return	autocal_result;								
					}					
					if((auto_cal_data_divide_1000 <= 1) && (auto_cal_data_divide_1000 >= -1))
					{
						dac_cal[outch].dac_0v_step = a&0x000000000000ffff;
						dac_cal[outch].dac_n0v_step = a&0x000000000000ffff; 
						dac_cal[outch].dac_0v_value = auto_cal_data_divide_1000;
						mode = DAC_CAL_N10V;
						printf("dac_cal[%d].dac_0v_step = %f / dac_cal[%d].dac_0v_value = %f\r\n", outch, dac_cal[outch].dac_0v_step, outch,dac_cal[outch].dac_0v_value);			
						break;
					}
					else if(auto_cal_data_divide_1000 < -1)
					{
						if(auto_cal_data_divide_1000 < -10)	
						{
							a = a + 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[+10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}					
						else if(auto_cal_data_divide_1000 < -1)	
						{
							a = a + 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[+1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					else if(auto_cal_data_divide_1000 > 1)
					{
						if(auto_cal_data_divide_1000 > 10)
						{
							a = a - 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[-10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);							
						}
						else if(auto_cal_data_divide_1000 > 1)
						{
							a = a - 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[-1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					count++;
				}	
				else if(mode == DAC_CAL_N10V)
				{
					if((auto_cal_data_divide_1000 > 0) || (auto_cal_data_divide_1000 < -12000) || (auto_cal_data_divide_1000 > -8000))
					{
						autocal_result = outch+1;
						return	autocal_result;								
					}						
					if((auto_cal_data_divide_1000 >= -10001) && (auto_cal_data_divide_1000 <= -9999))
					{
						dac_cal[outch].dac_n10v_step = a&0x000000000000ffff;
						dac_cal[outch].dac_n10v_value = auto_cal_data_divide_1000;
						mode = DAC_CAL_N25V;
						printf("dac_cal[%d].dac_n10v_step = %f / dac_cal[%d].dac_n10v_value = %f\r\n", outch, dac_cal[outch].dac_n10v_step, outch,dac_cal[outch].dac_n10v_value);			
						break;
					}
					else if(auto_cal_data_divide_1000 > -10000)
					{
						if(auto_cal_data_divide_1000 > -9990)	
						{
							a = a - 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[-10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}					
						else if(auto_cal_data_divide_1000 > -9999)	
						{
							a = a - 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[-1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					else if(auto_cal_data_divide_1000 < -10000)
					{
						if(auto_cal_data_divide_1000 < -10010)
						{
							a = a + 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[+10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);							
						}
						else if(auto_cal_data_divide_1000 < -10001)
						{
							a = a + 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[+1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					count++;
				}
				else if(mode == DAC_CAL_N25V)
				{
					if((auto_cal_data_divide_1000 > 0) ||(auto_cal_data_divide_1000 < -27000) || (auto_cal_data_divide_1000 > -23000))
					{
						autocal_result = outch+1;
						return	autocal_result;								
					}						
					if((auto_cal_data_divide_1000 >= -25001) && (auto_cal_data_divide_1000 <= -24999))
					{
						dac_cal[outch].dac_n25v_step = a&0x000000000000ffff;
						dac_cal[outch].dac_n25v_value = auto_cal_data_divide_1000;
						printf("dac_cal[%d].dac_n25v_step = %f / dac_cal[%d].dac_n25v_value = %f\r\n", outch, dac_cal[outch].dac_n25v_step, outch,dac_cal[outch].dac_n25v_value);			
						break;
					}
					else if(auto_cal_data_divide_1000 > -25000)
					{
						if(auto_cal_data_divide_1000 > -24990)	
						{
							a = a - 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[-10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}					
						else if(auto_cal_data_divide_1000 > -24999)	
						{
							a = a - 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;	
							dac_set(dac_num,c,3);					
							printf("[-1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					else if(auto_cal_data_divide_1000 < -25000)
					{
						if(auto_cal_data_divide_1000 < -25010)
						{
							a = a + 10;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[+10]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);							
						}
						else if(auto_cal_data_divide_1000 < -25001)
						{
							a = a + 1;
							c[2] = a &0xff;
							c[1] = (a>>8) &0xff;
							c[0] = 20+dac_ch;		
							dac_set(dac_num,c,3);				
							printf("[+1]DAC Dmax offset = %x / auto_cal_data_divide_1000 = %d\r\n", a,auto_cal_data_divide_1000);						
						}	
					}
					count++;
				}																					
				usleep(1000);
				if(count == DAC_AUTOCAL_MAX_COUNT)	
				{
					autocal_result = outch+1;
					power_off(); 
					return	autocal_result; 

				}
			}
		}
		if(mode==4)
		{
			dac_cal[outch].dac_0to10_ratio = (double)((dac_cal[outch].dac_10v_step - dac_cal[outch].dac_0v_step)) / (double)((dac_cal[outch].dac_10v_value - dac_cal[outch].dac_0v_value));
			dac_cal[outch].dac_10to25_ratio = (double)((dac_cal[outch].dac_25v_step - dac_cal[outch].dac_10v_step)) / (double)((dac_cal[outch].dac_25v_value - dac_cal[outch].dac_10v_value));
			dac_cal[outch].dac_0ton10_ratio = (double)((dac_cal[outch].dac_0v_step - dac_cal[outch].dac_n10v_step)) / (double)((dac_cal[outch].dac_0v_value - dac_cal[outch].dac_n10v_value));
			dac_cal[outch].dac_n10ton25_ratio =(double)((dac_cal[outch].dac_n10v_step - dac_cal[outch].dac_n25v_step)) / (double)((dac_cal[outch].dac_n10v_value - dac_cal[outch].dac_n25v_value));		

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
		power_off(); 
		return	autocal_result;	
	} 
	else
	{
		autocal_result = KEITHLEY_COM_ERROR;
		return	autocal_result;	
	}	
}

unsigned char dac_auto_offset_cal(int outch)
{
	unsigned char category_start_flag = 0xff;
	short a = 0;
	short vol = 0;
	int mode = 0;
	int i = 0;
	int ii = 0;
	int auto_cal_data_divide_1000 = 0;
	char dac_num;
	char dac_ch = 0;	
	unsigned char data[100];
	unsigned char ch = 0;
	unsigned char result = 0;
	int sum_data = 0;
	double avr_data = 0;
	int cycle = 0;

	unsigned char vol_cur_select_task_result = 0; 
    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    if(!vol_cur_select_task_result)
	{
		printf("ch[%d] CAL START\r\n", outch);
		memset(&data,0, sizeof(data));
		dac_cal[outch].dac_10to25_offset = 0;	
		dac_cal[outch].dac_0to10_offset = 0; 
		dac_cal[outch].dac_0ton10_offset = 0; 
		dac_cal[outch].dac_n10to_n25_offset = 0; 

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

		for(i = 0 ; i < DAC_OFFSET_N11V_N25V+1 ; i++)
		//for(i = 0 ; i < 1 ; i++)
		{			
			ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
			if(category_start_flag == DAC_ADC_AUTO_STOP)	
			{
				result = DAC_CAL_CANCEL;
				return	result;	
			}
			/*if(kbhit())	ch = readch();
			usleep(100000);
			if((ch == 'q') || (ch=='Q'))	
			{
				result = DAC_AUTOCHECK_CANCEL;
				return	result;			
			}*/		
			printf("mode = %d / dac_num = %d / outch = %d / dac_ch = %d\r\n", mode, dac_num, outch, dac_ch);	
			if(mode==DAC_OFFSET_11V_25V)	
			{
				printf("[DAC_OFFSET_11V_25V_START]\r\n");
				vol = 15000;
				cycle = 3;			
				ii = 0;
			}
			else if(mode==DAC_OFFSET_0V_10V) 
			{
				printf("[DAC_OFFSET_0V_10V_START]\r\n");
				vol = 5000;
				cycle = 3;			
				ii = 0;
			}
			else if(mode==DAC_OFFSET_0V_N10V) 
			{
				printf("[DAC_OFFSET_0V_N10V_START]\r\n");
				vol = -1000;
				cycle = 3;			
				ii = 0;
			}		
			else if(mode==DAC_OFFSET_N11V_N25V) 
			{
				printf("[DAC_OFFSET_N11V_N25V_START]\r\n");
				vol = -11000;
				cycle = 3;			
				ii = 0;
			}
			for(ii = 0 ; ii <cycle ; ii++)
			{
				ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
				if(category_start_flag == DAC_ADC_AUTO_STOP)	
				{
					result = DAC_CAL_CANCEL;
					return	result;	
				}
				signal_group.signal_config.dc_voltage[outch] = vol;	
				Power_Supply_Voltage_load();
				SET_DAC_OUTPUT_VALUE(outch);
				pattern_generator->PG_CONTROL = OUT_EN_ON;
				gpio_enble->GPIO_DATA |= (0x00000001<<(outch&0x3fff));
				if(outch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
				else if(outch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
				else if(outch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
				else if(outch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
				else if(outch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
				else if(outch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
				data[0] = AUTO_CAL_TASK;	
				data[1] = outch;	
				//usleep(100000);		
				ex_port_send_task(data,AUTO_CAL_TASK_LEN);				
				ex_port_serial_task(); 
        		//ex_recive_data_delay_task(1000);
				usleep(200000);	
				auto_cal_data_divide_1000 = auto_cal_data/1000; 
				sum_data += (vol - auto_cal_data_divide_1000);	
				printf("%d = %d - %d\r\n",sum_data,vol,auto_cal_data_divide_1000);
				vol -= 1000;	
				if(ii == cycle-1)	
				{				
					avr_data = (double)sum_data / cycle;
					if(mode==DAC_OFFSET_11V_25V)	
					{
						dac_cal[outch].dac_10to25_offset = avr_data * dac_cal[outch].dac_10to25_ratio;		
						printf("---avr_data = [%f]\r\n",avr_data);
						printf("---dac_cal[%d].dac_10to25_offset = [%f]\r\n",outch,dac_cal[outch].dac_10to25_offset);
						mode = DAC_OFFSET_0V_10V; 
					}
					else if(mode==DAC_OFFSET_0V_10V)	
					{
						dac_cal[outch].dac_0to10_offset = avr_data * dac_cal[outch].dac_0to10_ratio;	
						printf("---avr_data = [%f]\r\n",avr_data);	 
						printf("---dac_cal[%d].dac_0to10_offset = [%f]\r\n",outch,dac_cal[outch].dac_0to10_offset);
						mode = DAC_OFFSET_0V_N10V; 
					}
					else if(mode==DAC_OFFSET_0V_N10V)	
					{
						dac_cal[outch].dac_0ton10_offset = avr_data * dac_cal[outch].dac_0ton10_ratio;	
						printf("---avr_data = [%f]\r\n",avr_data);	
						printf("---dac_cal[%d].dac_0ton10_offset = [%f]\r\n",outch,dac_cal[outch].dac_0ton10_offset);
						mode = DAC_OFFSET_N11V_N25V; 
					}
					else if(mode==DAC_OFFSET_N11V_N25V)
					{
						dac_cal[outch].dac_n10to_n25_offset = avr_data * dac_cal[outch].dac_n10ton25_ratio;	
						printf("---avr_data = [%f]\r\n",avr_data);	 
						printf("---dac_cal[%d].dac_n10to_n25_offset = [%f]\r\n",outch,dac_cal[outch].dac_n10to_n25_offset);
					}
					avr_data = 0;
					sum_data = 0;
				}
			}
		}
	}
	else	result = KEITHLEY_COM_ERROR; 
	return	result;	 
}

unsigned char dac_vol_check(int outch)
{
	char ch = 0;
	short a = 0;
	short vol = 0;
	int i = 0;
	int check_data = 0;
	char dac_num;
	char dac_ch = 0;
	unsigned char data[100];
	unsigned char result = 0;
	unsigned char vol_cur_select_task_result = 0; 

	memset(&data,0, sizeof(data));	    
    vol_cur_select_task_result = vol_cur_select_task(SELECT_VOL);
    if(!vol_cur_select_task_result)
	{
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

		//vol = -25000;
		vol = 0;			
		for(i = 0 ; i < 51 ; i++)
		{
			if(kbhit())	ch = readch();
			
			if((ch == 'q') || (ch=='Q'))	
			{
				result = DAC_AUTOCHECK_CANCEL;
				return	result;			
			}			
			signal_group.signal_config.dc_voltage[outch] = vol;	
			Power_Supply_Voltage_load();
			SET_DAC_OUTPUT_VALUE(outch);
			pattern_generator->PG_CONTROL = OUT_EN_ON;
			gpio_enble->GPIO_DATA |= (0x00000001<<(outch&0x3fff));
			if(outch ==ELVDD)	gpiops_set_value(RELAY_ELVDD,RELAY_ON);
			else if(outch ==ADD8_S)	gpiops_set_value(RELAY_VDD8_S,RELAY_ON);
			else if(outch ==ADD8_G)	gpiops_set_value(RELAY_VDD8_G,RELAY_ON);
			else if(outch ==VGH)	gpiops_set_value(RELAY_VGH,RELAY_ON);
			else if(outch ==VGL)	gpiops_set_value(RELAY_VGL,RELAY_ON);	
			else if(outch ==VINIT)	gpiops_set_value(RELAY_VINIT,RELAY_ON);	
			data[0] = AUTO_CAL_TASK;	
			data[1] = outch;	
			ex_port_send_task(data,AUTO_CAL_TASK_LEN);		
			ex_port_serial_task(); 
			//ex_recive_data_delay_task(1000);	
			usleep(200000);			
			check_data = vol-auto_cal_data/1000; 
			if(cprf)	printf("[%d] = [%d] - [%d]\r\n", abs(check_data),vol,auto_cal_data/1000);
			if(abs(check_data) > 11)	
			{
				result = 1;	
				break;
			}
			else if(abs(check_data) > 15)	
			{
				result = 2;	
				break;
			}
			
			if(vol == 25000)	vol = 0;

			if(i >= 25) vol -= 1000;		
			else	vol += 1000; 	
		}
		
	}
	else	result = KEITHLEY_COM_ERROR; 
	return	result; 
}

