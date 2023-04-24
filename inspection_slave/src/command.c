#include "command.h"
#include "dac.h"
#include "drv.h"
#include "pg.h"
#include "model.h"
#include "adc.h"

#define COM_BUFFER_SIZE  1024

#define COMMAND_OFFSET		0
#define LENGTH_OFFSET			2
#define DATA_OFFSET				4

#define MODEL_CONFIG				0x0000
#define MODEL_CONFIG_ACK		0x0001
#define PATTERN_ID					0x0002
#define PATTERN_ID_ACK			0x0003
#define PATTERN_ON					0x0004
#define PATTERN_ON_ACK			0x0005
#define VOLTAGE_STATUS			0x0006
#define VOLTAGE_STATUS_ACK	0x0007
#define DYNAMIC_RGB				0x0008
#define DYNAMIC_RGB_ACK		0x0009
#define STATUS_LED					0x000A
#define STATUS_LED_ACK			0x000B

#define SPI_nCS_HIGH			*R_P5oHIGH = (1<<5)		
#define SPI_nCS_LOW				*R_P5oLOW = (1<<5)	
#define SPI_nRESET_HIGH		*R_P5oHIGH = (1<<0)		
#define SPI_nRESET_LOW		*R_P5oLOW = (1<<0)	
#define SPI_nWP_HIGH			*R_P5oHIGH = (1<<1)		
#define SPI_nWP_LOW			*R_P5oLOW = (1<<1)
#define SPI_SCLK_HIGH			*R_P5oHIGH = (1<<2)		
#define SPI_SCLK_LOW			*R_P5oLOW = (1<<2)
#define SPI_MOSI_HIGH			*R_P5oHIGH = (1<<3)		
#define SPI_MOSI_LOW			*R_P5oLOW = (1<<3)
#define SPI_MISO					(*R_P5iLEV >> 4) & 0X01

 void cmdint(char c);
 void cmdpop();
 void lexan(char *ptr);
 void debug_callback(char *debug_put);
 void com_callback(unsigned char *ch);
 void debug_task(char *ptr);
 void com_task(unsigned char *ptr);
 void com_ack(unsigned short command);
 void do_help(void);
 static void VOLT_ACK_MAPPING(COM_ACK_VOLT *voltage);

unsigned char debug_buffer[64] = {0};
unsigned char debug_put_cnt = 0;
unsigned char com_packet_state = 0;
unsigned short com_put_cnt = 0;
unsigned char com_put;
unsigned char rear_buffer[COM_BUFFER_SIZE] = {0};
unsigned char front_buffer[COM_BUFFER_SIZE] = {0};
U32 rear = 0;
U32 front = 0;

unsigned long long packet_check = (unsigned long long)0;
unsigned long long stx = (((unsigned long long)0x37a4c295) << 32) | (((unsigned long long)0x0000000037a4c295));
unsigned long long etx = (((unsigned long long)0x592c4a0d) << 32) | (((unsigned long long)0x00000000592c4a0d));

extern unsigned char LED_OP_FLAG;
	
void uart1_isr(void)
{
	while((*(volatile U32*)(((U32)R_UART1_BASE) + UART_LSR)) & ULSR_DRDY)//data ready
	{
		rear_buffer[rear++] = *(volatile U8*)(((U32)R_UART1_BASE)+UART_RBR);
		if(rear >= COM_BUFFER_SIZE) rear = 0;
	}
}	
	
void uart_task(void)
{
	char debug_ch;
	
	if(UartGetCh(DEBUG_UART, &debug_ch) != 0 ) 
	{
		debug_callback(&debug_ch);
	}
	//if(rear != front)
	while(rear != front)
	{
		com_callback(&rear_buffer[front++]);
		if(front >= COM_BUFFER_SIZE) front = 0;
	}
}

void debug_callback(char *ch)
{
	if(*ch == 13)
	{
		debugprintf("\r\n");
		if(debug_put_cnt != 0) debug_task(debug_buffer);
		debug_put_cnt = 0;
		memset(debug_buffer, 0, sizeof(debug_buffer));
		debugprintf(PROMPT);
	}
	else if(*ch == 0x08)
	{
		if(debug_put_cnt > 0)
		{
			UartPutCh(DEBUG_UART, 0x08);
			UartPutCh(DEBUG_UART, 0x20);
			UartPutCh(DEBUG_UART, 0x08);
			debug_buffer[--debug_put_cnt] = '\0';
		}
	}
	else
	{
		if(debug_put_cnt < 64)
		{
			debug_buffer[debug_put_cnt++] = *ch;
			UartPutCh(DEBUG_UART, *ch);			
		}
	}
}

void com_callback(unsigned char *ch)
{
	packet_check = ((packet_check << 8) | ((unsigned long long)(*ch) & ((unsigned long long)0x00000000000000FF)));
	
	if(packet_check == stx)
	{
		com_packet_state = 1;
		com_put_cnt = 0;
		//memset(front_buffer, 0, sizeof(front_buffer));
	}
	else if(packet_check == etx)
	{
		com_packet_state = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		com_task(front_buffer);
	}
	else
	{
		if(com_packet_state == 1) front_buffer[com_put_cnt++] = *ch;
		if(com_put_cnt >= COM_BUFFER_SIZE) com_packet_state = 0;
	}
}
	
void debug_task(char *ptr)
{
	if(!strncmp(ptr, "help", 4)) do_help();
	else if(!strncmp(ptr, "reset", 5)) swreset();
	else if(!strncmp(ptr, "regr ", 5))
	{
		unsigned int offset;
		unsigned short *addr = NULL;
		
		if((ptr + 5) != NULL)
		{
			offset = (unsigned int)atoi(ptr + 5);
			addr = (unsigned short *)(FPGA_BASE_ADDR + offset);
			debugprintf("Addr = %d, Read Data = 0x%x\r\n", (unsigned int)offset, (unsigned int)(*addr));
		}
	}
	else if(!strncmp(ptr, "regw ", 5))
	{
		unsigned int offset;
		unsigned short *addr = NULL;
		unsigned short data;
		char *temp_ptr = NULL;
		
		if((ptr + 5) != NULL)
		{
			temp_ptr = strtok(ptr + 5, ",");
			if(temp_ptr != NULL)
			{
				offset = (unsigned int) atoi(temp_ptr);
				addr = (unsigned short *)(FPGA_BASE_ADDR + offset);
				temp_ptr = strtok(NULL, ",");
				if(temp_ptr != NULL)
				{
					data = (unsigned short)strtol(temp_ptr, NULL, 16);
					*addr = data;
					debugprintf("Addr = %d, Write = 0x%x\r\n", (unsigned int)offset, (unsigned int)data);
				}
			}
		}
	}
	else if(!strncmp(ptr, "model save", 10))
	{
		if(MODEL_SAVE()) debugprintf("Model Save OK\r\n");
		else debugprintf("Model Save Fail\r\n");
	}
	else if(!strncmp(ptr, "model load", 10))
	{
		if(MODEL_LOAD()) debugprintf("Model Load OK\r\n");
		else debugprintf("Model Load Fail\r\n");
	}
	else if(!strncmp(ptr, "model print", 11))
	{
		MODEL_PRINT();
	}
	else if(!strncmp(ptr, "adc print", 9))
	{	
		int i;
		
		for(i=0; i<56; i++)
		{
			debugprintf("adc[%02d] = %d", i, (int)(adc_sensing_value.value[i]));
			if((i == 0) || (i == 5) || (i == 7) || (i == 12) || (i == 19)
			|| (i == 26) || (i == 32) || (i == 39) || (i == 46) || (i == 53))
			{
				debugprintf(" ** ");
			}
			else debugprintf("      ");
			if((i % 7) == 6) debugprintf("\r\n");
		}
	}
	else if(!strncmp(ptr, "dac cal", 7))
	{
		DAC_CALIBRATION();
	}
	else if(!strncmp(ptr, "outdelay ", 9))
	{
		if(ptr == NULL) return;
		output_delay = (unsigned int)atoi(ptr + 9);
		debugprintf("Output Delay = %u\r\n", output_delay);
		pg_handle->output_delay_l = (unsigned short)((output_delay >> 0) & 0x0000FFFF);
		pg_handle->output_delay_h = (unsigned short)((output_delay >> 16) & 0x0000FFFF); 
	}
	else if(!strncmp(ptr, "outdelayp", 9))
	{
		debugprintf("Output Delay = %u\r\n", (unsigned int)(((pg_handle->output_delay_l << 0) & 0x0000FFFF ) | ((pg_handle->output_delay_h << 16) & 0xFFFF0000 )));
	}
	else if(!strncmp(ptr, "set1", 4))
	{
		int i;
		for(i=3; i<12;i++) ensis_model_gp.pat_conf[0].sig_conf[i].volt_high = (short)5000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_high = (short)10000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_low = (short)-10000;
		if(!SET_DAC_OUTPUT_VALUE()) debugprintf("DAC Setting Fail\r\n");
		
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
		
		pg_handle->end_l = (unsigned short)0;
		pg_handle->end_h = (unsigned short)0;
		
		pg_handle->set_l = (unsigned short)0xFFFF;
		pg_handle->set_h = (unsigned short)0xFFFF;
		delayms(1);
		pg_handle->set_l = (unsigned short)0x0000;
		pg_handle->set_h = (unsigned short)0x0000;
	}
	else if(!strncmp(ptr, "set2", 4))
	{
		int i;
		for(i=3; i<12;i++) ensis_model_gp.pat_conf[0].sig_conf[i].volt_high = (short)-5000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_high = (short)10000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_low = (short)-10000;
		if(!SET_DAC_OUTPUT_VALUE()) debugprintf("DAC Setting Fail\r\n");
		
		pg_handle->vtotal_l = (unsigned short)1000;
		pg_handle->vtotal_h = (unsigned short)0;
	
		pg_handle->inversion_l = (unsigned short)0xFFFF;
		pg_handle->inversion_h = (unsigned short)0xFFFF;
		
		pg_handle->start_l = (unsigned short)0;
		pg_handle->start_h = (unsigned short)0;
		
		pg_handle->period_l = (unsigned short)1000;
		pg_handle->period_h = (unsigned short)0;
			
		pg_handle->delay_l = (unsigned short)1000;
		pg_handle->delay_h = (unsigned short)0;
		
		pg_handle->width_l = (unsigned short)0;
		pg_handle->width_h = (unsigned short)0;
		
		pg_handle->end_l = (unsigned short)0;
		pg_handle->end_h = (unsigned short)0;
		
		pg_handle->set_l = (unsigned short)0xFFFF;
		pg_handle->set_h = (unsigned short)0xFFFF;
		delayms(1);
		pg_handle->set_l = (unsigned short)0x0000;
		pg_handle->set_h = (unsigned short)0x0000;
	}
	else if(!strncmp(ptr, "set4", 4))
	{
		int i;
		for(i=3; i<12;i++) ensis_model_gp.pat_conf[0].sig_conf[i].volt_high = (short)-6900;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_high = (short)10000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_low = (short)-10000;
		if(!SET_DAC_OUTPUT_VALUE()) debugprintf("DAC Setting Fail\r\n");
		
		pg_handle->vtotal_l = (unsigned short)32000;
		pg_handle->vtotal_h = (unsigned short)0;
	
		pg_handle->inversion_l = (unsigned short)0xFFFF;
		pg_handle->inversion_h = (unsigned short)0xFFFF;
		
		pg_handle->start_l = (unsigned short)0;
		pg_handle->start_h = (unsigned short)0;
		
		pg_handle->period_l = (unsigned short)4000;
		pg_handle->period_h = (unsigned short)0;
			
		pg_handle->delay_l = (unsigned short)2000;
		pg_handle->delay_h = (unsigned short)0;
		
		pg_handle->width_l = (unsigned short)2000;
		pg_handle->width_h = (unsigned short)0;
		
		pg_handle->end_l = (unsigned short)32000;
		pg_handle->end_h = (unsigned short)0;
		
		pg_handle->set_l = (unsigned short)0xFFFF;
		pg_handle->set_h = (unsigned short)0xFFFF;
		delayms(1);
		pg_handle->set_l = (unsigned short)0x0000;
		pg_handle->set_h = (unsigned short)0x0000;
	}		
	else if(!strncmp(ptr, "on", 2))
	{
		pg_handle->control = (unsigned short)0x5E;
		delayms(100);
		pg_handle->control = (unsigned short)0x7E;
	}
	else if(!strncmp(ptr, "off", 3))
	{
		pg_handle->control = (unsigned short)0x1E;
		delayms(100);
		pg_handle->control = (unsigned short)0x3E;
	}
	else if(!strncmp(ptr, "r", 1))
	{
		rgb_handle.red = 3851;
		rgb_handle.green = 6700;
		rgb_handle.blue = 6700;
		
		if(VARIABLE_RGB() != TRUE) debugprintf("VARIABLE_RGB Fail\r\n");
		debugprintf("red\r\n");
	}
	else if(!strncmp(ptr, "g", 1))
	{
		rgb_handle.red = 6700;
		rgb_handle.green = 4225;
		rgb_handle.blue = 6700;
		
		if(VARIABLE_RGB() != TRUE) debugprintf("VARIABLE_RGB Fail\r\n");
		debugprintf("greenW\r\n");
	}
	else if(!strncmp(ptr, "b", 1))
	{
		rgb_handle.red = 6700;
		rgb_handle.green = 6700;
		rgb_handle.blue = 3406;
		
		if(VARIABLE_RGB() != TRUE) debugprintf("VARIABLE_RGB Fail\r\n");
		debugprintf("blue\r\n");
	}
	else if(!strncmp(ptr, "init", 4))
	{
		SPI_nRESET_HIGH;
		SPI_nCS_HIGH;
		SPI_MOSI_HIGH;
		SPI_SCLK_HIGH;	
		SPI_nWP_LOW;
		printf("FLASH INIT\r\n");
	}	
	else if(!strncmp(ptr, "read", 4))
	{
		unsigned int  i;
		unsigned int  ii;
		unsigned char data = 0;
		unsigned char rev = 0;
		unsigned short rev_d = 0;
	
		data = 0x05;		
		SPI_nCS_HIGH;
		SPI_nCS_LOW;	

		for(i = 0x80 ; i!= 0 ; i>>=1)
		{
			//printf("TP1 = %x\r\n",i);
			SPI_SCLK_HIGH;
			if(data & i)	SPI_MOSI_HIGH;
			else				SPI_MOSI_LOW;
			SPI_SCLK_LOW;
		}
		for(ii = 0 ; ii!=8 ; ii++)
		{
			//printf("TP2 = %x\r\n",ii);
			SPI_SCLK_LOW;
			rev = ((*R_P5iLEV >> 4) & 0x01);
			
			SPI_SCLK_HIGH;
			rev_d = (rev_d<<ii) | rev;
			printf("rev = %x // ii = %x //  rev_d = %x\r\n", rev,ii, rev_d);
		}
		//printf("TP100 = %x\r\n", rev);
			
	}		
	else debugprintf("Command Not Found\r\n");
}

 void com_task(unsigned char *ptr)
{
	unsigned short check_sum;
	int i;
	unsigned short loop_count;
	unsigned short flag;
	
	protocol.command = (unsigned short *)(ptr + COMMAND_OFFSET);
	protocol.length = (unsigned short *)(ptr + LENGTH_OFFSET) ;
	loop_count = (unsigned short)((*protocol.length) + DATA_OFFSET);
	protocol.data = (unsigned char *)(ptr + DATA_OFFSET);
	protocol.checksum = (unsigned short *)(ptr + DATA_OFFSET + (*(protocol.length)));
	
	if(*protocol.length > sizeof(ensis_model_gp.pat_conf))
	{
		debugprintf("Maybe RS232 Data Error\r\n");
		return ;
	}
	
	check_sum = 0;	
	for(i=0; i<loop_count; i++) check_sum = (check_sum + ((*(ptr + i)) & 0xff)) & 0xffff;
	
	if(check_sum == (*protocol.checksum))
	{
		/*
		debugprintf("command = %x\r\n", *protocol.command);
		debugprintf("length = %x\r\n", *protocol.length);
		debugprintf("checksum = %x\r\n", *protocol.checksum);
		debugprintf("check_sum = %x\r\n", check_sum);
		*/
		
		if((*protocol.command) == PATTERN_ID)
		{
			memset(&ensis_model_gp, 0, sizeof(ensis_model_gp));
			memcpy(&ensis_model_gp.pat_conf[0], protocol.data, (*protocol.length < sizeof(ensis_model_gp.pat_conf)) ? *protocol.length : sizeof(ensis_model_gp.pat_conf));
			if(!SET_DAC_OUTPUT_VALUE())
			{
				debugprintf("SET DAC OUTPUT VALUE Fail\r\n");
				return ;
			}
			
			if(!SET_PG_SIGNAL())
			{
				debugprintf("SET PG SIGNAL Fail\r\n");
				return;
			}
			
			flag = pg_handle->control | (PG_DAC_LOAD_OPERATION|PG_SIGNAL_INVERSION_CHANGE|PG_TIMING_CHANGE);
			if(!SET_PG_CONTROL(&flag))
			{
				debugprintf("SET PG CONTROL Fail\r\n");
				return;
			}
			delayms(1);
			com_ack(PATTERN_ID_ACK);
			debugprintf("PATTERN CHANGE\r\n");
			
			/*
			if(!MODEL_SAVE())
			{
				debugprintf("MODEL SAVE Fail\r\n");
				return;
			}*/
		}
		else if((*protocol.command) == PATTERN_ON)
		{
			if(*protocol.data == 1)
			{
				flag = pg_handle->control | (PG_DAC_LOAD_OPERATION|PG_SIGNAL_INVERSION_CHANGE|PG_TIMING_CHANGE|PG_ANALOG_MUX_ENABLE);
				if(!SET_PG_CONTROL(&flag))
				{
					debugprintf("SET PG CONTROL Fail\r\n");
					return;
				}
				delayms(1);
				com_ack(PATTERN_ON_ACK);
				debugprintf("Pattern ON\r\n");
			}
			else
			{
				flag = pg_handle->control & (~PG_ANALOG_MUX_ENABLE);
				if(!SET_PG_CONTROL(&flag))
				{
					debugprintf("SET PG CONTROL Fail\r\n");
					return;
				}
				delayms(1);
				com_ack(PATTERN_ON_ACK);
				debugprintf("Pattern OFF\r\n");
			}
		}
		else if((*protocol.command) == VOLTAGE_STATUS)
		{
			com_ack(VOLTAGE_STATUS_ACK);
		}
		else if((*protocol.command) == DYNAMIC_RGB)
		{
			memset(&rgb_handle, 0, sizeof(rgb_handle));
			memcpy(&rgb_handle, protocol.data, (*protocol.length < sizeof(rgb_handle)) ? *protocol.length : sizeof(rgb_handle));
			
			if(VARIABLE_RGB() == TRUE)
			{
				delayms(1);
				com_ack(DYNAMIC_RGB_ACK);
				debugprintf("VARIABLE_RGB \r\n");
			}
		}
		else if((*protocol.command) == STATUS_LED)
		{
			if(*protocol.data == 1) LED_OP_FLAG = 1;
			else LED_OP_FLAG = 0;
			delayms(1);
			com_ack(STATUS_LED_ACK);
		}
	}
	else debugprintf("Checksum Error\r\n");
}

void com_ack(unsigned short command)
{
	unsigned int i;
	unsigned int loop_count;
	unsigned short check_sum = 0;
	
	if(command == MODEL_CONFIG_ACK)
	{
		loop_count = 4;
		ack.command = command;
		ack.length = 0;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&ack)) + i + 8)) & 0xff) & 0xFFFF);
		ack.checksum = check_sum;

		loop_count = (unsigned int)sizeof(ack);
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i));
	}
	else if(command == PATTERN_ID_ACK)
	{
		loop_count = 4;
		ack.command = command;
		ack.length = 0;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&ack)) + i + 8)) & 0xff) & 0xFFFF);
		ack.checksum = check_sum;
		
		loop_count = (unsigned int)sizeof(ack);
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i));
	}
	else if(command == PATTERN_ON_ACK)
	{
		loop_count = 4;
		ack.command = command;
		ack.length = 0;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&ack)) + i + 8)) & 0xff) & 0xFFFF);
		ack.checksum = check_sum;
		
		loop_count = (unsigned int)sizeof(ack);
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i));
	}
	else if(command == VOLTAGE_STATUS_ACK)
	{
		COM_ACK_VOLT volt;

		VOLT_ACK_MAPPING(&volt);
		
		loop_count = 4;
		ack.command = command;
		ack.length = 66;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&ack)) + i + 8)) & 0xff) & 0xFFFF);
		loop_count = 66;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&volt)) + i)) & 0xff) & 0xFFFF);
		ack.checksum = check_sum;
		
		loop_count = ((unsigned int)sizeof(ack)) - 10;
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i));
		loop_count = (unsigned int)sizeof(volt);
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&volt)) + i));
		loop_count = (unsigned int)10;
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i + 12));
	}
	else if(command == DYNAMIC_RGB_ACK)
	{
		loop_count = 4;
		ack.command = command;
		ack.length = 0;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&ack)) + i + 8)) & 0xff) & 0xFFFF);
		ack.checksum = check_sum;
		
		loop_count = (unsigned int)sizeof(ack);
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i));
	}
	else if(command == STATUS_LED_ACK)
	{
		loop_count = 4;
		ack.command = command;
		ack.length = 0;
		for(i=0; i<loop_count; i++) check_sum += (((*(((unsigned char *)(&ack)) + i + 8)) & 0xff) & 0xFFFF);
		ack.checksum = check_sum;
		
		loop_count = (unsigned int)sizeof(ack);
		for(i=0; i<loop_count; i++) UartPutCh(COM_UART, *(((unsigned char *)(&ack)) + i));
	}
}
	
void do_help(void)
{
	debugprintf("01.FPGA Register Read = regr <Decimal Address>\r\n");
	debugprintf("02.FPGA Register Write = regw <Decimal Address>,<Hex Value>\r\n");
	debugprintf("03.Current Model SAVE = model save\r\n");
	debugprintf("04.Model LOAD at FLASH = model load\r\n");
	debugprintf("05.Model Information Print= model print\r\n");
	debugprintf("06.ADC Sensing Value Print = adc print\r\n");
	debugprintf("07.DAC Calibration Start = dac cal\r\n");
	debugprintf("08.Outdelay Setting = outdelay <Decimal Value>\r\n");
	debugprintf("09.Outdelay Print = outdelayp\r\n");
}

static void VOLT_ACK_MAPPING(COM_ACK_VOLT *voltage)
{
   voltage->voltage[0] = adc_sensing_value.value[21];                                                                                        //VOLT_DP1
   voltage->voltage[1] = adc_sensing_value.value[28];                                                                                        //VOLT_DP2                                                                                  
   voltage->voltage[2] = adc_sensing_value.value[35];                                                                                        //VOLT_DP3
   voltage->voltage[3] = adc_sensing_value.value[42];                                                                                        //VOLT_DP4
   voltage->voltage[4] = adc_sensing_value.value[49];                                                                                        //VOLT_DP5
   voltage->voltage[5] = adc_sensing_value.value[1];                                                                                         //VOLT_DP6
   voltage->voltage[6] = adc_sensing_value.value[8];                                                                                         //VOLT_DP7
   voltage->voltage[7] = adc_sensing_value.value[15];                                                                                        //VOLT_DP8
   voltage->voltage[8] = adc_sensing_value.value[22];                                                                                        //VOLT_DP9
   voltage->voltage[9] = adc_sensing_value.value[29];                                                                                        //VOLT_DIGITAL1
   voltage->voltage[10] = adc_sensing_value.value[36];                                                                                      //VOLT_DIGITAL2
   voltage->voltage[11] = adc_sensing_value.value[43];                                                                                      //VOLT_DIGITAL3
   voltage->voltage[12] = adc_sensing_value.value[50];                                                                                      //VOLT_DIGITAL4
   voltage->voltage[13] = adc_sensing_value.value[2];                                                                                        //VOLT_DIGITAL5
   voltage->voltage[14] = adc_sensing_value.value[9];                                                                                        //VOLT_DIGITAL6
   voltage->voltage[15] = adc_sensing_value.value[16];                                                                                      //VOLT_DIGITAL7
   voltage->voltage[16] = adc_sensing_value.value[23];                                                                                      //VOLT_DIGITAL8
   voltage->voltage[17] = adc_sensing_value.value[30];                                                                                      //VOLT_DIGITAL9
   voltage->voltage[18] = adc_sensing_value.value[37];                                                                                      //VOLT_DIGITAL10
   voltage->voltage[19] = adc_sensing_value.value[44];                                                                                      //VOLT_DIGITAL11
   voltage->voltage[20] = adc_sensing_value.value[51];                                                                                      //VOLT_DIGITAL12
   voltage->voltage[21] = adc_sensing_value.value[3];                                                                                        //VOLT_DIGITAL13
   voltage->voltage[22] = adc_sensing_value.value[10];                                                                                      //VOLT_DIGITAL14
   voltage->voltage[23] = adc_sensing_value.value[17];                                                                                      //VOLT_DIGITAL15
   voltage->voltage[24] = adc_sensing_value.value[24];                                                                                      //VOLT_DIGITAL16
   voltage->voltage[25] = adc_sensing_value.value[31];                                                                                      //VOLT_DIGITAL17
   voltage->voltage[26] = adc_sensing_value.value[38];                                                                                      //VOLT_DIGITAL18
   voltage->voltage[27] = adc_sensing_value.value[45];                                                                                      //VOLT_DIGITAL19
   voltage->voltage[28] = adc_sensing_value.value[52];                                                                                      //VOLT_DIGITAL20
   voltage->voltage[29] = adc_sensing_value.value[4];                                                                                        //VOLT_DIGITAL21
   voltage->voltage[30] = adc_sensing_value.value[11];                                                                                      //VOLT_DIGITAL22
   voltage->voltage[31] = adc_sensing_value.value[18];                                                                                      //VOLT_DIGITAL23
   voltage->voltage[32] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
}
