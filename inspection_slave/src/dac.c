#include "dac.h"
#include "cantus.h"
#include "command.h"
#include "pg.h"

#define CALIBRATION_FILE_FOLDER	("0:/calibration")
#define CALIBRATION_FILE_NAME 	("0:/calibration/calibration.dat")

void SET_DAC_OUTPUT_VALUE_INIT_FOR_CALIBRATION(void);
void SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION(int channel, short value);
BOOL CALIBRATION_SAVE(CALIBRATION *save_cal);
void CALIBRATION_SETTING(int set);
static BOOL DAC_REG_SETTING_VALUE_LOAD(DAC_REG_ADDRESS address, unsigned short *init_value);
	
unsigned short GET_DAC_STATUS(void)
{
	unsigned short result;
	
	result = dac_handle->status;
	
	return result;
}

BOOL SET_DAC_CONTROL(unsigned short *data)
{
	unsigned int i = 0;
	
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 1) return FALSE;
	}
	dac_handle->control = *data;
	return TRUE;
}

BOOL SET_DAC_SETTING_VALUE(DAC_SETTING *dac_setting)
{
	unsigned int i = 0;
	
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 1) return FALSE;
	}

	dac_handle->setting_buffer0 = dac_setting->data;
	dac_handle->setting_buffer1 = (dac_setting->address & 0x000F);
	dac_handle->setting_control = (1 << dac_setting->device_num);
	
	return TRUE;
}

//#define EP263_AC_MAGNIFICATION	(5.0*((double)3.35294118))
//#define EP263_AC_CALCULATION(x)	((short)((((double)x/1000.0) / EP263_AC_MAGNIFICATION) * 32767.0))
//#define EP264_DC_MAGNIFICATION	(5.0*((double)3.35294118))
//#define EP264_DC_CALCULATION(x)	((short)((((double)x/1000.0) / EP264_DC_MAGNIFICATION) * 32767.0))

BOOL SET_DAC_OUTPUT_VALUE(void)
{	
	int i;
	int ii = 2;
	int iii = 2;
	unsigned char pattern_id ;
	
	pattern_id = ensis_operation.pattern_id;
	for(i = 0 ; i <= 0x000a ; i++)
	{
		dac_handle->dac_select	=	(unsigned short)i;
		delayms(1);
		dac_handle->dac_out0 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[ii++].volt_low /1000.0) + 20));			
		dac_handle->dac_out1 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[iii++].volt_high /1000.0) + 20));
		dac_handle->dac_out2 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[ii++].volt_low /1000.0) + 20));
		dac_handle->dac_out3 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[iii++].volt_high /1000.0) + 20));
		dac_handle->dac_out4 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[ii++].volt_low /1000.0) + 20));
		dac_handle->dac_out5 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[iii++].volt_high /1000.0) + 20));
		dac_handle->dac_out6 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[ii++].volt_low /1000.0) + 20));
		dac_handle->dac_out7 = (unsigned short)((double)(65535/40)*(double)((ensis_model_gp.pat_conf[pattern_id].sig_conf[iii++].volt_high /1000.0) + 20));
	}
	
	
	i = 0;
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 1) return FALSE;
	}
	dac_handle->control = DAC_TX_START;
	
	return TRUE;
}

BOOL DAC_INIT(void)
{
	static unsigned short write_data;
	int i,j;
	int dac_i;
	
	i = 0;
	while(1)
	{
		if(dac_handle->status & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}
	
	for(dac_i=0x01; dac_i <0x10000 ; dac_i <<=1 )
	{
		for(j=0x00; j<0x10; j++)	
		{
			if(DAC_REG_SETTING_VALUE_LOAD((DAC_REG_ADDRESS)j, &write_data))
			{
				dac_handle->setting_buffer1 = (unsigned short)j;		
				dac_handle->setting_buffer0 = (unsigned short)write_data;
				dac_handle->setting_control = (unsigned short)dac_i;
				i = 0;
				while(1)
				{
					if(dac_handle->status & DAC_READY) break;
					delayms(1);
					i++;
					if(i > 5) return FALSE;				
				}
			}
		} 
	}
	
	i = 0;
	while(1)
	{
		if(dac_handle->status & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 5) return FALSE;
	}	
	return TRUE;
}

static BOOL DAC_REG_SETTING_VALUE_LOAD(DAC_REG_ADDRESS address, unsigned short *init_value)
{
	switch(address)
	{
 		case NOP:											//0x00
			return FALSE;
		case DEVICEID:									//0x01
			return FALSE;
		case DAC_STATUS:								//0x02
			return FALSE;
		case SPICONFIG:									//0x03
			*init_value = 0x0084;
			return TRUE;
		case GENCONFIG:								//0x04
			*init_value = 0x3F00;
			return TRUE;
		case BRDCONFIG:								//0x05
			*init_value = 0xF00F;
			return TRUE;			
		case SYNCCONFIG:								//0x06
			*init_value = 0x0FF0;
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
			*init_value = 0xCCCC;
			return TRUE;			
		case DACRANGE1:								//0x0C
			*init_value = 0xCCCC;
			return TRUE;		
		case TRIGGER:									//0x0E
			return FALSE;			
		case BRDCAST:									//0x0F
			return FALSE;			
		default:
			return FALSE; 
	}
}

void DAC_0V_SETTING(void)
{
	int i, ii;
	i = 0;
	
	for(ii = 0 ; ii <= 0x000a ; ii++)
	{
		dac_handle->dac_select	=	(unsigned short)ii;
		delayms(1);		
		dac_handle->dac_out0 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out1 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out2 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out3 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out4 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out5 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out6 = (unsigned short)((65535/40)* (0 + 20));
		dac_handle->dac_out7 = (unsigned short)((65535/40)* (0 + 20));
	}		
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 1) 
		{
			debugprintf("DAC_NG\r\n");
			break;
		}
	}
	dac_handle->control = DAC_TX_START; 
	
	i = 0;
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 1) 
		{
			debugprintf("DAC_NG\r\n");
			break;
		}
	}
	dac_handle->control = DAC_LOAD;		
	debugprintf("DAC 0V SETTING\r\n");
}


/* void DAC_CALIBRATION(void)
{
	int i;
	char temp_char;
	CALIBRATION temp_cal;
	short temp_value;
	
	CALIBRATION_SETTING(0);
	
	memset(&temp_cal, 0, sizeof(temp_cal));
	debugprintf("Calibration Start\r\n");
	for(i=0; i<143; i++)
	{
		SET_DAC_OUTPUT_VALUE_INIT_FOR_CALIBRATION();
		if((i % 11) == 0) temp_value = 29317;				//+15V
		else if((i % 11) == 1) temp_value = 23453;			//+12V
		else if((i % 11) == 2) temp_value = 17590;			//+9V
		else if((i % 11) == 3) temp_value = 11727;			//+6V
		else if((i % 11) == 4) temp_value = 5863;			//+3V
		else if((i % 11) == 5) temp_value = 0;			//0V
		else if((i % 11) == 6) temp_value = -5863;			//-3V
		else if((i % 11) == 7) temp_value = -11727;			//-6V
		else if((i % 11) == 8) temp_value = -17590;			//-9V
		else if((i % 11) == 9) temp_value = -23453;			//-12V
		else temp_value = -29317;		//-15V

		if(i < 99)
		{
			if((i % 11) == 0) debugprintf("DC[%02d] 15V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 1) debugprintf("DC[%02d] 12V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 2) debugprintf("DC[%02d] 9V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 3) debugprintf("DC[%02d] 6V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 4) debugprintf("DC[%02d] 3V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 5) debugprintf("DC[%02d] 0V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 6) debugprintf("DC[%02d] -3V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 7) debugprintf("DC[%02d] -6V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 8) debugprintf("DC[%02d] -9V = %d\r\n", i/11, temp_value);
			else if((i % 11) == 9) debugprintf("DC[%02d] -12V = %d\r\n", i/11, temp_value);
			else debugprintf("DC[%02d] -15V = %d\r\n", i/11, temp_value);
		}
		else if(i < 143)
		{
			if((i % 11) == 0) debugprintf("AC[%02d] 15V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 1) debugprintf("AC[%02d] 12V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 2) debugprintf("AC[%02d] 9V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 3) debugprintf("AC[%02d] 6V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 4) debugprintf("AC[%02d] 3V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 5) debugprintf("AC[%02d] 0V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 6) debugprintf("AC[%02d] -3V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 7) debugprintf("AC[%02d] -6V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 8) debugprintf("AC[%02d] -9V = %d\r\n", (i/11)-9, temp_value);
			else if((i % 11) == 9) debugprintf("AC[%02d] -12V = %d\r\n", (i/11)-9, temp_value);
			else debugprintf("AC[%02d] -15V = %d\r\n", (i/11)-9, temp_value);

			if((((i/11) - 9) == 0) || (((i/11) - 9) == 2)) CALIBRATION_SETTING(0); //delay output
			else CALIBRATION_SETTING(1); //width output
		}
		SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION((i/11), temp_value);
					
		while(1)
		{
			if(UartGetCh(DEBUG_UART, &temp_char) != 0)
			{
				if(temp_char == '=')//plus
				{
					if(temp_value < 32767) temp_value++;
					SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION((i/11), temp_value);
					if(i < 99)
					{
						if((i % 11) == 0) debugprintf("DC[%02d] 15V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 1) debugprintf("DC[%02d] 12V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 2) debugprintf("DC[%02d] 9V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 3) debugprintf("DC[%02d] 6V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 4) debugprintf("DC[%02d] 3V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 5) debugprintf("DC[%02d] 0V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 6) debugprintf("DC[%02d] -3V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 7) debugprintf("DC[%02d] -6V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 8) debugprintf("DC[%02d] -9V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 9) debugprintf("DC[%02d] -12V = %d\r\n", i/11, temp_value);
						else debugprintf("DC[%02d] -15V = %d\r\n", i/11, temp_value);	
					}
					else if(i < 143)
					{	
						if((i % 11) == 0) debugprintf("AC[%02d] 15V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 1) debugprintf("AC[%02d] 12V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 2) debugprintf("AC[%02d] 9V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 3) debugprintf("AC[%02d] 6V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 4) debugprintf("AC[%02d] 3V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 5) debugprintf("AC[%02d] 0V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 6) debugprintf("AC[%02d] -3V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 7) debugprintf("AC[%02d] -6V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 8) debugprintf("AC[%02d] -9V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 9) debugprintf("AC[%02d] -12V = %d\r\n", (i/11)-9, temp_value);
						else debugprintf("AC[%02d] -15V = %d\r\n", (i/11)-9, temp_value);
					}
				}
				else if(temp_char == '-')//minus
				{
					if(temp_value > -32768) temp_value--;
					SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION((i/11), temp_value);
					if(i < 99)
					{
						if((i % 11) == 0) debugprintf("DC[%02d] 15V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 1) debugprintf("DC[%02d] 12V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 2) debugprintf("DC[%02d] 9V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 3) debugprintf("DC[%02d] 6V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 4) debugprintf("DC[%02d] 3V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 5) debugprintf("DC[%02d] 0V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 6) debugprintf("DC[%02d] -3V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 7) debugprintf("DC[%02d] -6V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 8) debugprintf("DC[%02d] -9V = %d\r\n", i/11, temp_value);
						else if((i % 11) == 9) debugprintf("DC[%02d] -12V = %d\r\n", i/11, temp_value);
						else debugprintf("DC[%02d] -15V = %d\r\n", i/11, temp_value);	
					}
					else if(i < 143)
					{	
						if((i % 11) == 0) debugprintf("AC[%02d] 15V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 1) debugprintf("AC[%02d] 12V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 2) debugprintf("AC[%02d] 9V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 3) debugprintf("AC[%02d] 6V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 4) debugprintf("AC[%02d] 3V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 5) debugprintf("AC[%02d] 0V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 6) debugprintf("AC[%02d] -3V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 7) debugprintf("AC[%02d] -6V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 8) debugprintf("AC[%02d] -9V = %d\r\n", (i/11)-9, temp_value);
						else if((i % 11) == 9) debugprintf("AC[%02d] -12V = %d\r\n", (i/11)-9, temp_value);
						else debugprintf("AC[%02d] -15V = %d\r\n", (i/11)-9, temp_value);
					}
				}
				else if(temp_char == 13)//enter
				{
					if(i < 99)
					{
						if((i % 11) == 0)
						{
							temp_cal.dc[i/11].high_15V = temp_value;
							debugprintf("DC[%02d] 15V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 1)
						{
							temp_cal.dc[i/11].high_12V = temp_value;
							debugprintf("DC[%02d] 12V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 2)
						{
							temp_cal.dc[i/11].high_9V = temp_value;
							debugprintf("DC[%02d] 9V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 3)
						{
							temp_cal.dc[i/11].high_6V = temp_value;
							debugprintf("DC[%02d] 6V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 4)
						{
							temp_cal.dc[i/11].high_3V = temp_value;
							debugprintf("DC[%02d] 3V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 5)
						{
							temp_cal.dc[i/11].zero = temp_value;
							debugprintf("DC[%02d] 0V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 6)
						{
							temp_cal.dc[i/11].low_3V = temp_value;
							debugprintf("DC[%02d] -3V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 7)
						{
							temp_cal.dc[i/11].low_6V = temp_value;
							debugprintf("DC[%02d] -6V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 8)
						{
							temp_cal.dc[i/11].low_9V = temp_value;
							debugprintf("DC[%02d] -9V = %d\r\n", i/11, temp_value);
						}
						else if((i % 11) == 9)
						{
							temp_cal.dc[i/11].low_12V = temp_value;
							debugprintf("DC[%02d] -12V = %d\r\n", i/11, temp_value);
						}
						else
						{
							temp_cal.dc[i/11].low_15V = temp_value;
							debugprintf("DC[%02d] -15V = %d\r\n", i/11, temp_value);
						}
					}
					else if(i < 143)
					{
						if((i % 11) == 0)
						{
							temp_cal.ac[(i/11)-9].high_15V = temp_value;
							debugprintf("AC[%02d] 15V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 1)
						{
							temp_cal.ac[(i/11)-9].high_12V = temp_value;
							debugprintf("AC[%02d] 12V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 2)
						{
							temp_cal.ac[(i/11)-9].high_9V = temp_value;
							debugprintf("AC[%02d] 9V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 3)
						{
							temp_cal.ac[(i/11)-9].high_6V = temp_value;
							debugprintf("AC[%02d] 6V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 4)
						{
							temp_cal.ac[(i/11)-9].high_3V = temp_value;
							debugprintf("AC[%02d] 3V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 5)
						{
							temp_cal.ac[(i/11)-9].zero = temp_value;
							debugprintf("AC[%02d] 0V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 6)
						{
							temp_cal.ac[(i/11)-9].low_3V = temp_value;
							debugprintf("AC[%02d] -3V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 7)
						{
							temp_cal.ac[(i/11)-9].low_6V = temp_value;
							debugprintf("AC[%02d] -6V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 8)
						{
							temp_cal.ac[(i/11)-9].low_9V = temp_value;
							debugprintf("AC[%02d] -9V = %d\r\n", (i/11)-9, temp_value);
						}
						else if((i % 11) == 9)
						{
							temp_cal.ac[(i/11)-9].low_12V = temp_value;
							debugprintf("AC[%02d] -12V = %d\r\n", (i/11)-9, temp_value);
						}
						else
						{
							temp_cal.ac[(i/11)-9].low_15V = temp_value;
							debugprintf("AC[%02d] -15V = %d\r\n", (i/11)-9, temp_value);
						}
					}
					break;
				}
				else if((temp_char == 'q') || (temp_char == 'Q'))//quit
				{
					debugprintf("Force Quit\r\n");
					pg_handle->control = (unsigned short)0x1E;
					delayms(100);
					pg_handle->control = (unsigned short)0x3E;
					return;
				}
			}
		}
	}
	
	if(CALIBRATION_SAVE(&temp_cal))
	{
		memcpy(&cal, &temp_cal, sizeof(temp_cal));
		debugprintf("Calibration Done\r\n");
	}
	else debugprintf("Calibration Save Fail\r\n");
	
	pg_handle->control = (unsigned short)0x1E;
	delayms(100);
	pg_handle->control = (unsigned short)0x3E;
	system_init();
} */
/*
void DAC_CALIBRATION(void)
{
	int i;
	char temp_char;
	CALIBRATION temp_cal;
	short temp_value;
	
	CALIBRATION_SETTING(0);
	
	memset(&temp_cal, 0, sizeof(temp_cal));
	debugprintf("Calibration Start\r\n");
	for(i=0; i<39; i++)
	{
		SET_DAC_OUTPUT_VALUE_INIT_FOR_CALIBRATION();
		if((i % 3) == 0) temp_value = 29317;
		else if((i % 3) == 1) temp_value = 0;
		else temp_value = -29317;
		
		if(i < 27)
		{
			if((i % 3) == 0) debugprintf("DC[%02d].max = %d\r\n", i/3, temp_value);
			else if((i % 3) == 1) debugprintf("DC[%02d].zero = %d\r\n", i/3, temp_value);
			else debugprintf("DC[%02d].min = %d\r\n", i/3, temp_value);
		}
		else if(i < 39)
		{
			if((i % 3) == 0) debugprintf("AC[%02d].max = %d\r\n", (i/3)-9, temp_value);
			else if((i % 3) == 1) debugprintf("AC[%02d].zero = %d\r\n", (i/3)-9, temp_value);
			else debugprintf("AC[%02d].min = %d\r\n", (i/3)-9, temp_value);
			
			if((((i/3) - 9) == 0) || (((i/3) - 9) == 2)) CALIBRATION_SETTING(0);
			else CALIBRATION_SETTING(1);
		}
		SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION((i/3), temp_value);
					
		while(1)
		{
			if(UartGetCh(DEBUG_UART, &temp_char) != 0)
			{
				if(temp_char == '=')//plus
				{
					if(temp_value < 32767) temp_value++;
					SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION((i/3), temp_value);
					if(i < 27)
					{
						if((i % 3) == 0) debugprintf("DC[%02d].max = %d\r\n", i/3, temp_value);
						else if((i % 3) == 1) debugprintf("DC[%02d].zero = %d\r\n", i/3, temp_value);
						else debugprintf("DC[%02d].min = %d\r\n", i/3, temp_value);
					}
					else if(i < 39)
					{
						if((i % 3) == 0) debugprintf("AC[%02d].max = %d\r\n", (i/3)-9, temp_value);
						else if((i % 3) == 1) debugprintf("AC[%02d].zero = %d\r\n", (i/3)-9, temp_value);
						else debugprintf("AC[%02d].min = %d\r\n", (i/3)-9, temp_value);
					}
				}
				else if(temp_char == '-')//minus
				{
					if(temp_value > -32768) temp_value--;
					SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION((i/3), temp_value);
					if(i < 27)
					{
						if((i % 3) == 0) debugprintf("DC[%02d].max = %d\r\n", i/3, temp_value);
						else if((i % 3) == 1) debugprintf("DC[%02d].zero = %d\r\n", i/3, temp_value);
						else debugprintf("DC[%02d].min = %d\r\n", i/3, temp_value);
					}
					else if(i < 39)
					{
						if((i % 3) == 0) debugprintf("AC[%02d].max = %d\r\n", (i/3)-9, temp_value);
						else if((i % 3) == 1) debugprintf("AC[%02d].zero = %d\r\n", (i/3)-9, temp_value);
						else debugprintf("AC[%02d].min = %d\r\n", (i/3)-9, temp_value);
					}
				}
				else if(temp_char == 13)//enter
				{
					if(i < 27)
					{
						if((i % 3) == 0)
						{
							temp_cal.dc[i/3].max = temp_value;
							debugprintf("DC[%02d].max = %d\r\n", i/3, temp_value);
						}
						else if((i % 3) == 1)
						{
							temp_cal.dc[i/3].zero = temp_value;
							debugprintf("DC[%02d].zero = %d\r\n", i/3, temp_value);
						}
						else
						{
							temp_cal.dc[i/3].min = temp_value;
							debugprintf("DC[%02d].min = %d\r\n", i/3, temp_value);
						}
					}
					else if(i < 39)
					{
						if((i % 3) == 0)
						{
							temp_cal.ac[(i/3)-9].max = temp_value;
							debugprintf("AC[%02d].max = %d\r\n", (i/3)-9, temp_value);
						}
						else if((i % 3) == 1)
						{
							temp_cal.ac[(i/3)-9].zero = temp_value;
							debugprintf("AC[%02d].zero = %d\r\n", (i/3)-9, temp_value);
						}
						else
						{
							temp_cal.ac[(i/3)-9].min = temp_value;
							debugprintf("AC[%02d].min = %d\r\n", (i/3)-9, temp_value);
						}
					}
					break;
				}
				else if((temp_char == 'q') || (temp_char == 'Q'))//quit
				{
					debugprintf("Force Quit\r\n");
					pg_handle->control = (unsigned short)0x1E;
					delayms(100);
					pg_handle->control = (unsigned short)0x3E;
					return;
				}
			}
		}
	}
	
	if(CALIBRATION_SAVE(&temp_cal))
	{
		memcpy(&cal, &temp_cal, sizeof(temp_cal));
		debugprintf("Calibration Done\r\n");
	}
	else debugprintf("Calibration Save Fail\r\n");
	
	pg_handle->control = (unsigned short)0x1E;
	delayms(100);
	pg_handle->control = (unsigned short)0x3E;
	system_init();
}
*/
/* void SET_DAC_OUTPUT_VALUE_INIT_FOR_CALIBRATION(void)
{
	unsigned short flag;
	
	dac_handle->dac0_out0 = (short)0;		//VOUT_DIGITAL1~8_LOW
	dac_handle->dac0_out1 = (short)0;			//VOUT_DIGITAL1~8_HIGH
	dac_handle->dac0_out2 = (short)0;		//VOUT_DIGITAL9~24_LOW
	dac_handle->dac0_out3 = (short)0;			//VOUT_DIGITAL9~24_HIGH
	
	dac_handle->dac1_out0 = (short)0;			//VOUT_DP1
	dac_handle->dac1_out1 = (short)0;			//VOUT_DP2
	dac_handle->dac1_out2 = (short)0;			//VOUT_DP3
	dac_handle->dac1_out3 = (short)0;			//VOUT_DP4

	dac_handle->dac2_out0 = (short)0;			//VOUT_DP5
	dac_handle->dac2_out1 = (short)0;			//VOUT_DP6
	dac_handle->dac2_out2 = (short)0;			//VOUT_DP7 // BLUE
	dac_handle->dac2_out3 = (short)0;		//VOUT_DP8 // GREEN
	
	dac_handle->dac3_out0 = (short)0;		//VOUT_DP9 //RED
	dac_handle->dac3_out1 = (short)0;		//UNUSED
	dac_handle->dac3_out2 = (short)0;		//UNUSED
	dac_handle->dac3_out3 = (short)0;		//UNUSED
	
	dac_handle->dac4_out0 = (short)0;		//TSP_TX(UNUSED)
	dac_handle->dac4_out1 = (short)0;		//TXP_RX(UNUSED)
	dac_handle->dac4_out2 = (short)0;		//UNUSED
	dac_handle->dac4_out3 = (short)0;		//UNUSED
	
	flag = DAC_TX_START;
	if(!SET_DAC_CONTROL(&flag)) debugprintf("DAC TX START FAIL\r\n");
	flag = DAC_LOAD;
	if(!SET_DAC_CONTROL(&flag)) debugprintf("DAC LOAD FAIL\r\n");
} */

/* void SET_DAC_OUTPUT_VALUE_FOR_CALIBRATION(int channel, short value)
{
	unsigned short flag;
	unsigned int i = 0;
	short *set_value = &value;
	
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 5)
		{
			debugprintf("DAC Not Ready\r\n");
			return;
		}
	}

	if(channel == 0) dac_handle->dac1_out0 = *((short *)set_value);//VOUT_DP1
	else if(channel == 1) dac_handle->dac1_out1 = *((short *)set_value);//VOUT_DP2
	else if(channel == 2) dac_handle->dac1_out2 = *((short *)set_value);//VOUT_DP3
	else if(channel == 3) dac_handle->dac1_out3 = *((short *)set_value);//VOUT_DP4
	else if(channel == 4) dac_handle->dac2_out0 = *((short *)set_value);//VOUT_DP5
	else if(channel == 5) dac_handle->dac2_out1 = *((short *)set_value);//VOUT_DP6
	else if(channel == 6) dac_handle->dac2_out2 = *((short *)set_value);//VOUT_DP7//BLUE
	else if(channel == 7) dac_handle->dac2_out3 = *((short *)set_value);//VOUT_DP8//GREEN
	else if(channel == 8) dac_handle->dac3_out0 = *((short *)set_value);//VOUT_DP9//RED
	else if(channel == 9) dac_handle->dac0_out0 = *((short *)set_value);//VOUT_DIGITAL1~8_LOW
	else if(channel == 10) dac_handle->dac0_out1 = *((short *)set_value);//VOUT_DIGITAL1~8_HIGH
	else if(channel == 11) dac_handle->dac0_out2 = *((short *)set_value);//VOUT_DIGITAL9~24_LOW
	else if(channel == 12) dac_handle->dac0_out3 = *((short *)set_value);//VOUT_DIGITAL9~24_HIGH
	else
	{
		debugprintf("Channel Error\r\n");
		return;
	}
	flag = DAC_TX_START;
	if(!SET_DAC_CONTROL(&flag)) debugprintf("DAC TX START FAIL\r\n");
	flag = DAC_LOAD;
	if(!SET_DAC_CONTROL(&flag)) debugprintf("DAC LOAD FAIL\r\n");
} */

/* BOOL CALIBRATION_SAVE(CALIBRATION *save_cal)
{
	FATFS fs;
	FIL fsrc;
	FIL *fp = &fsrc;
	DIR dir;
	FRESULT result;
	unsigned int write_byte;
	
	if(f_mount(DRIVE_NAND, &fs) != FR_OK)
	{
		debugprintf("Mount Error\r\n");
		return FALSE;
	}
	
	result = f_opendir(&dir, CALIBRATION_FILE_FOLDER);
	if(result != FR_OK) f_mkdir(CALIBRATION_FILE_FOLDER);
	
	result = f_open(fp, CALIBRATION_FILE_NAME, FA_CREATE_ALWAYS|FA_WRITE);
	if(result != FR_OK)
	{
		debugprintf("f_open Error\r\n");
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	
	result = f_write(fp, save_cal, sizeof(cal), &write_byte);
	if(result != FR_OK)
	{
		debugprintf("f_write Error\r\n");
		f_close(fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	debugprintf("%s %dbyte write ok\r\n", CALIBRATION_FILE_NAME, write_byte);
	
	result = f_close(fp);
	if(result != FR_OK)
	{
		f_close(fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	
	return TRUE;
} */

/* BOOL CALIBRATION_LOAD(void)
{
	FATFS fs;
	FIL fp;
	FRESULT result;
	unsigned int read_byte;
	
	if(f_mount(DRIVE_NAND, &fs) != FR_OK)
	{
		debugprintf("Mount Error\r\n");
		return FALSE;
	}
	
	result = f_open(&fp, CALIBRATION_FILE_NAME, FA_READ|FA_OPEN_EXISTING);
	if(result != FR_OK)
	{
		debugprintf("f_open Error\r\n");
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	
	result = f_read(&fp, &cal, sizeof(cal), &read_byte);
	if(result != FR_OK)
	{
		debugprintf("f_read Error\r\n");
		f_close(&fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	debugprintf("%s %dbyte read ok\r\n", CALIBRATION_FILE_NAME, read_byte);
	
	result = f_close(&fp);
	if(result != FR_OK)
	{
		f_close(&fp);
		f_mount(DRIVE_NAND,NULL);
		return FALSE;
	}
	return TRUE;
} */

/* void CALIBRATION_PRINT(void)
{
	int i;
	
	for(i=0; i<13; i++)
	{
		if(i < 9)
		{
			debugprintf("DC[%02d] 15V = %d\r\n", i, cal.dc[i].high_15V);
			debugprintf("DC[%02d] 12V = %d\r\n", i, cal.dc[i].high_12V);
			debugprintf("DC[%02d] 9V = %d\r\n", i, cal.dc[i].high_9V);
			debugprintf("DC[%02d] 6V = %d\r\n", i, cal.dc[i].high_6V);
			debugprintf("DC[%02d] 3V = %d\r\n", i, cal.dc[i].high_3V);
			debugprintf("DC[%02d] 0V = %d\r\n", i, cal.dc[i].zero);
			debugprintf("DC[%02d] -3V = %d\r\n", i, cal.dc[i].low_3V);
			debugprintf("DC[%02d] -6V = %d\r\n", i, cal.dc[i].low_6V);
			debugprintf("DC[%02d] -9V = %d\r\n", i, cal.dc[i].low_9V);
			debugprintf("DC[%02d] -12V = %d\r\n", i, cal.dc[i].low_12V);
			debugprintf("DC[%02d] -15V = %d\r\n", i, cal.dc[i].low_15V);
		}
		else
		{
			debugprintf("AC[%02d] 15V = %d\r\n", i-9, cal.ac[i-9].high_15V);
			debugprintf("AC[%02d] 12V = %d\r\n", i-9, cal.ac[i-9].high_12V);
			debugprintf("AC[%02d] 9V = %d\r\n", i-9, cal.ac[i-9].high_9V);
			debugprintf("AC[%02d] 6V = %d\r\n", i-9, cal.ac[i-9].high_6V);
			debugprintf("AC[%02d] 3V = %d\r\n", i-9, cal.ac[i-9].high_3V);
			debugprintf("AC[%02d] 0V = %d\r\n", i-9, cal.ac[i-9].zero);
			debugprintf("AC[%02d] -3V = %d\r\n", i-9, cal.ac[i-9].low_3V);
			debugprintf("AC[%02d] -6V = %d\r\n", i-9, cal.ac[i-9].low_6V);
			debugprintf("AC[%02d] -9V = %d\r\n", i-9, cal.ac[i-9].low_9V);
			debugprintf("AC[%02d] -12V = %d\r\n", i-9, cal.ac[i-9].low_12V);
			debugprintf("AC[%02d] -15V = %d\r\n", i-9, cal.ac[i-9].low_15V);
		}
	}
} */

/* void CALIBRATION_SETTING(int set)
{
	if(!set)
	{
		pg_handle->vtotal_l = (unsigned short)1000;
		pg_handle->vtotal_h = (unsigned short)0;

		pg_handle->inversion_l = (unsigned short)0x0000;
		pg_handle->inversion_h = (unsigned short)0x0000;
		
		pg_handle->start_l = (unsigned short)0;
		pg_handle->start_h = (unsigned short)0;
		
		pg_handle->period_l = (unsigned short)1000;
		pg_handle->period_h = (unsigned short)0;
			
		pg_handle->delay_l = (unsigned short)1000;
		pg_handle->delay_h = (unsigned short)0;
		
		pg_handle->width_l = (unsigned short)0;
		pg_handle->width_h = (unsigned short)0;
		
		pg_handle->end_l = (unsigned short)1000;
		pg_handle->end_h = (unsigned short)0;
		
		pg_handle->set_l = (unsigned short)0xFFFF;
		pg_handle->set_h = (unsigned short)0xFFFF;
		delayms(1);
		pg_handle->set_l = (unsigned short)0x0000;
		pg_handle->set_h = (unsigned short)0x0000;
			
		pg_handle->control = (unsigned short)0x5E;
		delayms(100);
		pg_handle->control = (unsigned short)0x7E;
	}
	else
	{
		pg_handle->vtotal_l = (unsigned short)1000;
		pg_handle->vtotal_h = (unsigned short)0;

		pg_handle->inversion_l = (unsigned short)0x0000;
		pg_handle->inversion_h = (unsigned short)0x0000;
		
		pg_handle->start_l = (unsigned short)0;
		pg_handle->start_h = (unsigned short)0;
		
		pg_handle->period_l = (unsigned short)1000;
		pg_handle->period_h = (unsigned short)0;
			
		pg_handle->delay_l = (unsigned short)0;
		pg_handle->delay_h = (unsigned short)0;
		
		pg_handle->width_l = (unsigned short)1000;
		pg_handle->width_h = (unsigned short)0;
		
		pg_handle->end_l = (unsigned short)1000;
		pg_handle->end_h = (unsigned short)0;
		
		pg_handle->set_l = (unsigned short)0xFFFF;
		pg_handle->set_h = (unsigned short)0xFFFF;
		delayms(1);
		pg_handle->set_l = (unsigned short)0x0000;
		pg_handle->set_h = (unsigned short)0x0000;
			
		pg_handle->control = (unsigned short)0x5E;
		delayms(100);
		pg_handle->control = (unsigned short)0x7E;
	}
} */





/* BOOL SET_DAC_OUTPUT_VALUE(void)
{
	int i;
	unsigned char pattern_id ;
	
	pattern_id = ensis_operation.pattern_id;

	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= 12000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].high_15V) - ((double)cal.ac[0].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 12.0)) + ((double)cal.ac[0].high_12V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].high_15V) - ((double)cal.ac[2].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 12.0)) + ((double)cal.ac[2].high_12V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= 9000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].high_12V) - ((double)cal.ac[0].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 9.0)) + ((double)cal.ac[0].high_9V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].high_12V) - ((double)cal.ac[2].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 9.0)) + ((double)cal.ac[2].high_9V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= 6000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].high_9V) - ((double)cal.ac[0].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 6.0)) + ((double)cal.ac[0].high_6V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].high_9V) - ((double)cal.ac[2].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 6.0)) + ((double)cal.ac[2].high_6V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= 3000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].high_6V) - ((double)cal.ac[0].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 3.0)) + ((double)cal.ac[0].high_3V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].high_6V) - ((double)cal.ac[2].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 3.0)) + ((double)cal.ac[2].high_3V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= 0){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].high_3V) - ((double)cal.ac[0].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 0.0)) + ((double)cal.ac[0].zero));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].high_3V) - ((double)cal.ac[2].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) - 0.0)) + ((double)cal.ac[2].zero));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= -3000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].zero) - ((double)cal.ac[0].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 3.0)) + ((double)cal.ac[0].low_3V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].zero) - ((double)cal.ac[2].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 3.0)) + ((double)cal.ac[2].low_3V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= -6000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].low_3V) - ((double)cal.ac[0].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 6.0)) + ((double)cal.ac[0].low_6V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].low_3V) - ((double)cal.ac[2].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 6.0)) + ((double)cal.ac[2].low_6V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= -9000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].low_6V) - ((double)cal.ac[0].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 9.0)) + ((double)cal.ac[0].low_9V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].low_6V) - ((double)cal.ac[2].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 9.0)) + ((double)cal.ac[2].low_9V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low >= -12000){
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].low_9V) - ((double)cal.ac[0].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 12.0)) + ((double)cal.ac[0].low_12V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].low_9V) - ((double)cal.ac[2].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 12.0)) + ((double)cal.ac[2].low_12V));
	}
	else{
		dac_handle->dac0_out0 = (short)((((((double)cal.ac[0].low_12V) - ((double)cal.ac[0].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 15.0)) + ((double)cal.ac[0].low_15V));
		dac_handle->dac0_out2 = (short)((((((double)cal.ac[2].low_12V) - ((double)cal.ac[2].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_low) / 1000.0) + 15.0)) + ((double)cal.ac[2].low_15V));
	}
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= 12000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].high_15V) - ((double)cal.ac[1].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 12.0)) + ((double)cal.ac[1].high_12V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].high_15V) - ((double)cal.ac[3].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 12.0)) + ((double)cal.ac[3].high_12V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= 9000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].high_12V) - ((double)cal.ac[1].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 9.0)) + ((double)cal.ac[1].high_9V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].high_12V) - ((double)cal.ac[3].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 9.0)) + ((double)cal.ac[3].high_9V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= 6000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].high_9V) - ((double)cal.ac[1].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 6.0)) + ((double)cal.ac[1].high_6V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].high_9V) - ((double)cal.ac[3].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 6.0)) + ((double)cal.ac[3].high_6V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= 3000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].high_6V) - ((double)cal.ac[1].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 3.0)) + ((double)cal.ac[1].high_3V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].high_6V) - ((double)cal.ac[3].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 3.0)) + ((double)cal.ac[3].high_3V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= 0){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].high_3V) - ((double)cal.ac[1].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 0.0)) + ((double)cal.ac[1].zero));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].high_3V) - ((double)cal.ac[3].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) - 0.0)) + ((double)cal.ac[3].zero));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= -3000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].zero) - ((double)cal.ac[1].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 3.0)) + ((double)cal.ac[1].low_3V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].zero) - ((double)cal.ac[3].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 3.0)) + ((double)cal.ac[3].low_3V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= -6000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].low_3V) - ((double)cal.ac[1].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 6.0)) + ((double)cal.ac[1].low_6V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].low_3V) - ((double)cal.ac[3].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 6.0)) + ((double)cal.ac[3].low_6V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= -9000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].low_6V) - ((double)cal.ac[1].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 9.0)) + ((double)cal.ac[1].low_9V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].low_6V) - ((double)cal.ac[3].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 9.0)) + ((double)cal.ac[3].low_9V));
	}
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high >= -12000){
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].low_9V) - ((double)cal.ac[1].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 12.0)) + ((double)cal.ac[1].low_12V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].low_9V) - ((double)cal.ac[3].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 12.0)) + ((double)cal.ac[3].low_12V));
	}
	else{
		dac_handle->dac0_out1 = (short)((((((double)cal.ac[1].low_12V) - ((double)cal.ac[1].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 15.0)) + ((double)cal.ac[1].low_15V));
		dac_handle->dac0_out3 = (short)((((((double)cal.ac[3].low_12V) - ((double)cal.ac[3].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[12].volt_high) / 1000.0) + 15.0)) + ((double)cal.ac[3].low_15V));
	}
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= 12000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].high_15V) - ((double)cal.dc[0].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[0].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= 9000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].high_12V) - ((double)cal.dc[0].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[0].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= 6000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].high_9V) - ((double)cal.dc[0].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[0].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= 3000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].high_6V) - ((double)cal.dc[0].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[0].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= 0)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].high_3V) - ((double)cal.dc[0].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[0].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= -3000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].zero) - ((double)cal.dc[0].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[0].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= -6000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].low_3V) - ((double)cal.dc[0].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[0].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= -9000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].low_6V) - ((double)cal.dc[0].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[0].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high >= -12000)
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].low_9V) - ((double)cal.dc[0].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[0].low_12V));
	else
		dac_handle->dac1_out0 = (short)((((((double)cal.dc[0].low_12V) - ((double)cal.dc[0].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[3].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[0].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= 12000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].high_15V) - ((double)cal.dc[1].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[1].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= 9000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].high_12V) - ((double)cal.dc[1].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[1].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= 6000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].high_9V) - ((double)cal.dc[1].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[1].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= 3000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].high_6V) - ((double)cal.dc[1].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[1].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= 0)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].high_3V) - ((double)cal.dc[1].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[1].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= -3000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].zero) - ((double)cal.dc[1].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[1].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= -6000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].low_3V) - ((double)cal.dc[1].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[1].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= -9000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].low_6V) - ((double)cal.dc[1].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[1].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high >= -12000)
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].low_9V) - ((double)cal.dc[1].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[1].low_12V));
	else
		dac_handle->dac1_out1 = (short)((((((double)cal.dc[1].low_12V) - ((double)cal.dc[1].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[4].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[1].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= 12000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].high_15V) - ((double)cal.dc[2].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[2].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= 9000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].high_12V) - ((double)cal.dc[2].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[2].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= 6000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].high_9V) - ((double)cal.dc[2].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[2].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= 3000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].high_6V) - ((double)cal.dc[2].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[2].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= 0)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].high_3V) - ((double)cal.dc[2].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[2].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= -3000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].zero) - ((double)cal.dc[2].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[2].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= -6000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].low_3V) - ((double)cal.dc[2].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[2].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= -9000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].low_6V) - ((double)cal.dc[2].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[2].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high >= -12000)
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].low_9V) - ((double)cal.dc[2].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[2].low_12V));
	else
		dac_handle->dac1_out2 = (short)((((((double)cal.dc[2].low_12V) - ((double)cal.dc[2].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[5].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[2].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= 12000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].high_15V) - ((double)cal.dc[3].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[3].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= 9000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].high_12V) - ((double)cal.dc[3].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[3].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= 6000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].high_9V) - ((double)cal.dc[3].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[3].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= 3000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].high_6V) - ((double)cal.dc[3].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[3].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= 0)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].high_3V) - ((double)cal.dc[3].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[3].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= -3000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].zero) - ((double)cal.dc[3].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[3].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= -6000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].low_3V) - ((double)cal.dc[3].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[3].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= -9000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].low_6V) - ((double)cal.dc[3].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[3].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high >= -12000)
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].low_9V) - ((double)cal.dc[3].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[3].low_12V));
	else
		dac_handle->dac1_out3 = (short)((((((double)cal.dc[3].low_12V) - ((double)cal.dc[3].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[6].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[3].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= 12000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].high_15V) - ((double)cal.dc[4].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[4].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= 9000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].high_12V) - ((double)cal.dc[4].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[4].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= 6000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].high_9V) - ((double)cal.dc[4].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[4].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= 3000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].high_6V) - ((double)cal.dc[4].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[4].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= 0)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].high_3V) - ((double)cal.dc[4].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[4].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= -3000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].zero) - ((double)cal.dc[4].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[4].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= -6000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].low_3V) - ((double)cal.dc[4].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[4].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= -9000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].low_6V) - ((double)cal.dc[4].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[4].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high >= -12000)
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].low_9V) - ((double)cal.dc[4].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[4].low_12V));
	else
		dac_handle->dac2_out0 = (short)((((((double)cal.dc[4].low_12V) - ((double)cal.dc[4].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[7].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[4].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= 12000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].high_15V) - ((double)cal.dc[5].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[5].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= 9000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].high_12V) - ((double)cal.dc[5].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[5].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= 6000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].high_9V) - ((double)cal.dc[5].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[5].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= 3000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].high_6V) - ((double)cal.dc[5].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[5].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= 0)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].high_3V) - ((double)cal.dc[5].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[5].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= -3000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].zero) - ((double)cal.dc[5].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[5].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= -6000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].low_3V) - ((double)cal.dc[5].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[5].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= -9000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].low_6V) - ((double)cal.dc[5].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[5].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high >= -12000)
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].low_9V) - ((double)cal.dc[5].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[5].low_12V));
	else
		dac_handle->dac2_out1 = (short)((((((double)cal.dc[5].low_12V) - ((double)cal.dc[5].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[8].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[5].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= 12000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].high_15V) - ((double)cal.dc[6].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[6].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= 9000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].high_12V) - ((double)cal.dc[6].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[6].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= 6000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].high_9V) - ((double)cal.dc[6].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[6].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= 3000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].high_6V) - ((double)cal.dc[6].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[6].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= 0)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].high_3V) - ((double)cal.dc[6].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[6].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= -3000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].zero) - ((double)cal.dc[6].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[6].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= -6000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].low_3V) - ((double)cal.dc[6].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[6].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= -9000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].low_6V) - ((double)cal.dc[6].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[6].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high >= -12000)
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].low_9V) - ((double)cal.dc[6].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[6].low_12V));
	else
		dac_handle->dac2_out2 = (short)((((((double)cal.dc[6].low_12V) - ((double)cal.dc[6].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[9].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[6].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= 12000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].high_15V) - ((double)cal.dc[7].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[7].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= 9000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].high_12V) - ((double)cal.dc[7].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[7].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= 6000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].high_9V) - ((double)cal.dc[7].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[7].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= 3000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].high_6V) - ((double)cal.dc[7].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[7].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= 0)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].high_3V) - ((double)cal.dc[7].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[7].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= -3000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].zero) - ((double)cal.dc[7].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[7].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= -6000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].low_3V) - ((double)cal.dc[7].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[7].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= -9000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].low_6V) - ((double)cal.dc[7].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[7].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high >= -12000)
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].low_9V) - ((double)cal.dc[7].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[7].low_12V));
	else
		dac_handle->dac2_out3 = (short)((((((double)cal.dc[7].low_12V) - ((double)cal.dc[7].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[10].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[7].low_15V));
	
	if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= 12000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].high_15V) - ((double)cal.dc[8].high_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) - 12.0)) + ((double)cal.dc[8].high_12V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= 9000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].high_12V) - ((double)cal.dc[8].high_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) - 9.0)) + ((double)cal.dc[8].high_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= 6000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].high_9V) - ((double)cal.dc[8].high_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) - 6.0)) + ((double)cal.dc[8].high_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= 3000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].high_6V) - ((double)cal.dc[8].high_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) - 3.0)) + ((double)cal.dc[8].high_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= 0)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].high_3V) - ((double)cal.dc[8].zero)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) - 0.0)) + ((double)cal.dc[8].zero));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= -3000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].zero) - ((double)cal.dc[8].low_3V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) + 3.0)) + ((double)cal.dc[8].low_3V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= -6000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].low_3V) - ((double)cal.dc[8].low_6V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) + 6.0)) + ((double)cal.dc[8].low_6V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= -9000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].low_6V) - ((double)cal.dc[8].low_9V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) + 9.0)) + ((double)cal.dc[8].low_9V));
	else if(ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high >= -12000)
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].low_9V) - ((double)cal.dc[8].low_12V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) + 12.0)) + ((double)cal.dc[8].low_12V));
	else
		dac_handle->dac3_out0 = (short)((((((double)cal.dc[8].low_12V) - ((double)cal.dc[8].low_15V)) / 3.0) * ((((double)ensis_model_gp.pat_conf[pattern_id].sig_conf[11].volt_high) / 1000.0) + 15.0)) + ((double)cal.dc[8].low_15V));
	
	dac_handle->dac3_out1 = 0;		//UNUSED
	dac_handle->dac3_out2 = 0;		//UNUSED
	dac_handle->dac3_out3 = 0;		//UNUSED
	
	dac_handle->dac4_out0 = 0;		//TSP_TX(UNUSED)
	dac_handle->dac4_out1 = 0;		//TXP_RX(UNUSED)
	dac_handle->dac4_out2 = 0;		//UNUSED
	dac_handle->dac4_out3 = 0;		//UNUSED

	i = 0;
	while(1)
	{
		if(GET_DAC_STATUS() & DAC_READY) break;
		delayms(1);
		i++;
		if(i > 1) return FALSE;
	}
	dac_handle->control = DAC_TX_START;
	
	return TRUE;
}
 */
