#include "command.h"
#include "dac.h"
#include "drv.h"
#include "pg.h"
#include "model.h"
#include "adc.h"

#define COM_BUFFER_SIZE  1500

#define COMMAND_OFFSET		0
#define LENGTH_OFFSET			2
#define DATA_OFFSET				4

/* #define MODEL_CONFIG				0x0000
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
#define STATUS_LED_ACK			0x000B */

#define COMMAND_PAT_SET ((unsigned short)0x0001)
#define COMMAND_PAT_SET_ACK ((unsigned short)0x0002)
#define COMMAND_PAT_ONOFF ((unsigned short)0x0003)
#define COMMAND_PAT_ONOFF_ACK ((unsigned short)0x0004)
#define COMMAND_STATUS_REQ ((unsigned short)0x0005)
#define COMMAND_STATUS_REQ_ACK ((unsigned short)0x0006)
#define COMMAND_DYNAMIC_SIGNAL ((unsigned short)0x0007)
#define COMMAND_DYNAMIC_SIGNAL_ACK ((unsigned short)0x0008)
#define COMMAND_VERSION_REQ ((unsigned short)0x0009)
#define COMMAND_VERSION_REQ_ACK ((unsigned short)0x000A)



 void cmdint(char c);
 void cmdpop();
 void lexan(char *ptr);
 void debug_callback(char *debug_put);
 void com_callback(unsigned char *ch);
 void debug_task(char *ptr);
 void com_task(unsigned char *data, unsigned short length);
 void com_ack(unsigned short command);
 static void com_ack_send(unsigned short cmd, unsigned char *data, int length);
 static void com_ack_send_elp_protocol(unsigned short cmd, unsigned char *data, int length);
 void do_help(void);
 static void VOLT_ACK_MAPPING(COM_ACK_VOLT *voltage);

 
 unsigned char	packet_read[256] = {0};
  //unsigned char	packet_read_1k[2048] = {0};
 
 w25q32_t w25q32;

unsigned char debug_buffer[64] = {0};
unsigned char debug_put_cnt = 0;
unsigned char com_packet_state = 0;
unsigned short com_put_cnt = 0;
unsigned char com_put;
unsigned char rear_buffer[COM_BUFFER_SIZE] = {0};
unsigned char front_buffer[COM_BUFFER_SIZE] = {0};
U32 rear = 0;
U32 front = 0;
	
#define ENSIS_STX 0x37a4c295
#define ENSIS_ETX 0x592c4a0d
#define ELP_STX 0x95c2a437
#define ELP_ETX 0x0d4a2c59

unsigned int packet_check = 0;
	
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
	packet_check = ((packet_check << 8) | ((unsigned int)*ch));
	
	if(packet_check == ENSIS_STX)
	{
		com_packet_state = 1;
		com_put_cnt = 0;
	}
	else if((packet_check == ENSIS_ETX) & (com_packet_state == 1))
	{
		int i;
		com_packet_state = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		com_task(front_buffer, com_put_cnt);
/* 		for(i=0; i<com_put_cnt; i++)
			debugprintf("TP[%02d] = %x\r\n", i, front_buffer[i]); */
	}
	else if(packet_check == ELP_STX)
	{
		com_packet_state = 2;
		com_put_cnt = 0;
	}
	else if((packet_check == ELP_ETX) & (com_packet_state == 2))
	{
		int i;
		
		com_packet_state = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		front_buffer[--com_put_cnt] = 0;
		
		//수정필요 COM_UART를 ELP Port로 보내야함
		if(front_buffer[0] == 0){
			for(i=1; i<com_put_cnt; i++) UartPutCh(COM_UART, *((char *)(front_buffer+i)));
		}
		else if(front_buffer[0] == 1){
			for(i=1; i<com_put_cnt; i++) UartPutCh(COM_UART, *((char *)(front_buffer+i)));
		}
	}
	else
	{
		//if(com_packet_state == 1) front_buffer[com_put_cnt++] = *ch;
		if(com_packet_state) front_buffer[com_put_cnt++] = *ch;
		if(com_put_cnt >= COM_BUFFER_SIZE) com_packet_state = 0;
	}
}

	
void debug_task(char *ptr)
{
	if(!strncmp(ptr, "help", 4)) do_help();
	else if(!strncmp(ptr, "reset", 5))
	{
		DAC_0V_SETTING();	
		delayms(1);
		swreset();
	}
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
	else if(!strncmp(ptr, "init", 4))
	{
		flash_init();
		debugprintf("FLASH INIT\r\n");
	}
	else if(!strncmp(ptr, "wrien", 5))
	{
		write_enable();
	}
	else if(!strncmp(ptr, "wridn", 5))
	{
		write_disable();
	}
	else if(!strncmp(ptr, "qqq", 3))
	{
		unsigned int flash_read_address=0;
		unsigned int  i;
		unsigned int file_size = 0;
		unsigned char dd = 0;
		flash_read_address = FLASH_MEMORY_BLOCK0;	
		
		while(1)
		{
			 spi_read(READ_DATA, (unsigned int)flash_read_address,256);
			for(i = 0 ; i < 256 ; i++)
			{
				dd = packet_read[i];
				file_size++;
				debugprintf("%x\r\n",dd);

			}
			flash_read_address += 256;
			if(file_size > FPGA_FILE_SIZE+10)
			{
				debugprintf("FPGA_DOWNLOAD_FAIL\r\n");
				break;
			}
		}   
	}
	else if(!strncmp(ptr, "eee", 3))
	{
		DAC_INIT();
		debugprintf("DAC_INIT\r\n");
	}
	else if(!strncmp(ptr, "www", 3))
	{
 		debugprintf(" FPGA STATUS= %x\r\n",dac_handle->status);
 		debugprintf(" FPGA CONTROL= %x\r\n",dac_handle->control);
 		debugprintf(" FPGA Setting_Control= %x\r\n",dac_handle->setting_control);
 		debugprintf(" FPGA Setting_buffer0= %x\r\n",dac_handle->setting_buffer0);	
 		debugprintf(" FPGA Setting_buffer1= %x\r\n",dac_handle->setting_buffer1);					
 		debugprintf(" FPGA DA0= %x\r\n",dac_handle->dac_out0);
		debugprintf(" FPGA DA1= %x\r\n",dac_handle->dac_out1);
		debugprintf(" FPGA DA2= %x\r\n",dac_handle->dac_out2);
		debugprintf(" FPGA DA3= %x\r\n",dac_handle->dac_out3);
		debugprintf(" FPGA DA4= %x\r\n",dac_handle->dac_out4);
		debugprintf(" FPGA DA5= %x\r\n",dac_handle->dac_out5);
		debugprintf(" FPGA DA6= %x\r\n",dac_handle->dac_out6);
		debugprintf(" FPGA DA7= %x\r\n",dac_handle->dac_out7); 		
	
	}	
	else if(!strncmp(ptr, "sss", 3))
	{ 
		int i;
		i = 0;
		
		dac_handle->dac_select	=	(unsigned short)0x0000;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10 + 20));		
	
		dac_handle->dac_select	=	(unsigned short)0x0001;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10 + 20));	
		
		dac_handle->dac_select	=	(unsigned short)0x0002;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10 + 20));
		
		
		dac_handle->dac_select	=	(unsigned short)0x0003;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10+ 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10+ 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)*(10+ 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)*(-10+ 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (-10+ 20));
		
		dac_handle->dac_select	=	(unsigned short)0x0004;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10+ 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10+ 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10+ 20));
		
		dac_handle->dac_select	=	(unsigned short)0x0005;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* ( -10+ 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* ( 10+ 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* ( -10+ 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* ( 10+ 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* ( -10+ 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* ( 10+ 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* ( -10+ 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* ( 10+ 20));
		
		dac_handle->dac_select	=	(unsigned short)0x0006;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10+ 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10 + 20));
		
		dac_handle->dac_select	=	(unsigned short)0x0007;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10 + 20));
		
		dac_handle->dac_select	=	(unsigned short)0x0008;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* ( -10+ 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* ( -10+ 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10 + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10+ 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10 + 20));		
		
		dac_handle->dac_select	=	(unsigned short)0x0009;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10  + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10  + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10  + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10  + 20));	
		
		dac_handle->dac_select	=	(unsigned short)0x00a;		
		(dac_handle->dac_out0) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out1) = (unsigned short)((65535/40)* (10  + 20));
		(dac_handle->dac_out2) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out3) = (unsigned short)((65535/40)* (10  + 20));
		(dac_handle->dac_out4) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out5) = (unsigned short)((65535/40)* (10  + 20));
		(dac_handle->dac_out6) = (unsigned short)((65535/40)* (-10 + 20));
		(dac_handle->dac_out7) = (unsigned short)((65535/40)* (10  + 20));		
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
			
	}
	else if(!strncmp(ptr, "wqw", 3))
	{		
		ensis_operation.pattern_id = 0;
		
		memset(&ensis_model_gp, 0, sizeof(ensis_model_gp));
		ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[0]	=  (unsigned short)200;
		ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[1]	=  (unsigned short)200;
		ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[2]	=  (unsigned short)0;
		ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[3]	=  (unsigned short)0;
		ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4]	=  400;
		ensis_model_gp.pat_conf[0].sig_conf[2].inversion = 0;
		
	}
	else if(!strncmp(ptr, "zxz", 3))
	{
			pg_handle->start_l = (unsigned short)0; //START
			pg_handle->start_h = (unsigned short)0;			
			
			pg_handle->step1_low_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[0] >> 0) & 0x0000FFFF); //STEP1
			pg_handle->step1_low_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[0] >> 16) & 0x0000FFFF);
		
			pg_handle->step2_high_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[1] >> 0) & 0x0000FFFF); //STEP2
			pg_handle->step2_high_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[1] >> 16) & 0x0000FFFF);
			
			pg_handle->step3_low_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[2] >> 0) & 0x0000FFFF); //STEP3
			pg_handle->step3_low_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[2] >> 16) & 0x0000FFFF);
		
			pg_handle->step4_high_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[3] >> 0) & 0x0000FFFF); //STEP4
			pg_handle->step4_high_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[3] >> 16) & 0x0000FFFF);
		
			pg_handle->period_l = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 0) & 0x0000FFFF); //PERIOD
			pg_handle->period_h = (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 16) & 0x0000FFFF);	
		
			pg_handle->end_l =(unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 0) & 0x0000FFFF); 
			pg_handle->end_h =  (unsigned short)((ensis_model_gp.pat_conf[0].sig_conf[2].time_sector[4] >> 16) & 0x0000FFFF);	
			
			pg_handle->set_l = (unsigned short)0x0001;
			pg_handle->set_m = (unsigned short)0;			
			pg_handle->set_h = (unsigned short)0; 			
		//SET_PG_SIGNAL();
	}
	else if(!strncmp(ptr, "sds", 3))
	{ 
		ensis_model_gp.pat_conf[0].sig_conf[2].volt_low	=	(short)0 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[2].volt_high	=	(short)1 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[3].volt_low	=	(short)2 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[3].volt_high	=	(short)3 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[4].volt_low	=	(short)4 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[4].volt_high	=	(short)5 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[5].volt_low	=	(short)6 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[5].volt_high	=	(short)7 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[6].volt_low	=	(short)8 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[6].volt_high	=	(short)9 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[7].volt_low	=	(short)10 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[7].volt_high	=	(short)11 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[8].volt_low	=	(short)12 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[8].volt_high	=	(short)13 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[9].volt_low	=	(short)14 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[9].volt_high	=	(short)15 * 1000;	
		ensis_model_gp.pat_conf[0].sig_conf[10].volt_low	=	(short)0 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[10].volt_high	=	(short)-1 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[11].volt_low	=	(short)-2 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[11].volt_high	=	(short)-3 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_low	=	(short)-4 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[12].volt_high	=	(short)-5 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[13].volt_low	=	(short)-6 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[13].volt_high	=	(short)-7 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[14].volt_low	=	(short)-8 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[14].volt_high	=	(short)-9 * 1000;	
		ensis_model_gp.pat_conf[0].sig_conf[15].volt_low	=	(short)-10 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[15].volt_high	=	(short)-11 * 1000;		
		ensis_model_gp.pat_conf[0].sig_conf[16].volt_low	=	(short)-12 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[16].volt_high	=	(short)-13 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[17].volt_low	=	(short)-14 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[17].volt_high	=	(short)-15 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[18].volt_low	=	(short)0 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[18].volt_high	=	(short)1* 1000;
		ensis_model_gp.pat_conf[0].sig_conf[19].volt_low	=	(short)2 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[19].volt_high	=	(short)3 * 1000;	
		ensis_model_gp.pat_conf[0].sig_conf[20].volt_low	=	(short)4 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[20].volt_high	=	(short)5 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[21].volt_low	=	(short)6 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[21].volt_high	=	(short)7 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[22].volt_low	=	(short)8 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[22].volt_high	=	(short)9 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[23].volt_low	=	(short)10 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[23].volt_high	=	(short)11 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[24].volt_low	=	(short)12 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[24].volt_high	=	(short)13 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[25].volt_low	=	(short)14 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[25].volt_high	=	(short)15 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[26].volt_low	=	(short)0 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[26].volt_high	=	(short)-1 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[27].volt_low	=	(short)-2 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[27].volt_high	=	(short)-3 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[28].volt_low	=	(short)-4 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[28].volt_high	=	(short)-5 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[29].volt_low	=	(short)-6 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[29].volt_high	=	(short)-7 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[30].volt_low	=	(short)-8 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[30].volt_high	=	(short)-9 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[31].volt_low	=	(short)-10 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[31].volt_high	=	(short)-11 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[32].volt_low	=	(short)-12 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[32].volt_high	=	(short)-13 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[33].volt_low	=	(short)-14 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[33].volt_high	=	(short)-15 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[34].volt_low	=	(short)0 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[34].volt_high	=	(short)1 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[35].volt_low	=	(short)2 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[35].volt_high	=	(short)3 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[36].volt_low	=	(short)4 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[36].volt_high	=	(short)5 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[37].volt_low	=	(short)6 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[37].volt_high	=	(short)7 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[38].volt_low	=	(short)8 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[38].volt_high	=	(short)9 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[39].volt_low	=	(short)10 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[39].volt_high	=	(short)11 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[40].volt_low	=	(short)12 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[40].volt_high	=	(short)13 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[41].volt_low	=	(short)14 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[41].volt_high	=	(short)15 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[42].volt_low	=	(short)0 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[42].volt_high	=	(short)-1 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[43].volt_low	=	(short)-2 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[43].volt_high	=	(short)-3 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[44].volt_low	=	(short)-4 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[44].volt_high	=	(short)-5 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[45].volt_low	=	(short)-6 * 1000;
		ensis_model_gp.pat_conf[0].sig_conf[45].volt_high	=	(short)-7 * 1000;		
	}
	else if(!strncmp(ptr, "dsd", 3))
	{ 
		SET_DAC_OUTPUT_VALUE();
		debugprintf("DAC SETTING OK\r\n");
	}
	else if(!strncmp(ptr, "qwq", 3))
	{
		int i;
		unsigned short	inversion_data_0 = 0;
		unsigned short	inversion_data_1 = 0;
		unsigned short	inversion_data_2 = 0;		
		
 		for(i=0; i<NUMBER_OF_SIGNAL; i++)
		{
			if(i < 16)
			{
				if(ensis_model_gp.pat_conf[0].sig_conf[i].inversion) inversion_data_0 |= (1<<(i));	
			}
			else if(i >= 16 && i < 32)
			{
				if(ensis_model_gp.pat_conf[0].sig_conf[i].inversion) inversion_data_1 |= (1<<(i-16));
			}
			else	if(i >=32)
			{
				if(ensis_model_gp.pat_conf[0].sig_conf[i].inversion) inversion_data_2 |= (1<<(i-32));		
			}		
						
			pg_handle->inversion_l = inversion_data_0;
			pg_handle->inversion_m = inversion_data_1;
			pg_handle->inversion_h = inversion_data_2;
			delayms(1);
			debugprintf("TP1 =%d\r\n",i);			
			debugprintf(" FPGA0 set_l= %x\r\n",	pg_handle->inversion_l );		
			debugprintf(" FPGA0 set_m= %x\r\n",	pg_handle->inversion_m);		
			debugprintf(" FPGA0 set_h= %x\r\n",	pg_handle->inversion_h);						
		}		 			
	}
	else if(!strncmp(ptr, "dacval", 6))
	{
		dac_handle->dac_select	=	(unsigned short)0x0000;	
		debugprintf(" DA0_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf(" DA0_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf(" DA1_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf(" DA1_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf(" DA2_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf(" DA2_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf(" DA3_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf(" DA3_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);			
		debugprintf("---------------------------------------------------------------\r\n");
		dac_handle->dac_select	=	(unsigned short)0x0001;	
		debugprintf(" DA4_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf(" DA4_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf(" DA5_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf(" DA5_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf(" DA6_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf(" DA6_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf(" DA7_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf(" DA7_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0002;	
		debugprintf(" DA8_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf(" DA8_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf(" DA9_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf(" DA9_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf(" DA10_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf(" DA10_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf(" DA11_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf(" DA11_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0003;	
		debugprintf(" DA12_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf(" DA12_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf(" DA13_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf(" DA13_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf(" DA14_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf(" DA14_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf(" DA15_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf(" DA15_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0004;	
		debugprintf(" DA16_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf(" DA16_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf(" DA17_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf(" DA17_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf(" DA18_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf(" DA18_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf(" DA19_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf(" DA19_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0005;	
		debugprintf("  DA20_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf("  DA20_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf("  DA21_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf("  DA21_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf("  DA22_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf("  DA22_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf("  DA23_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf("  DA23_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);
		debugprintf("---------------------------------------------------------------\r\n");	
		dac_handle->dac_select	=	(unsigned short)0x0006;	
		debugprintf("  DA24_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf("  DA24_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf("  DA25_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf("  DA25_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf("  DA26_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf("  DA26_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf("  DA27_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf("  DA27_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0007;	
		debugprintf("  DA28_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf("  DA28_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf("  DA29_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf("  DA29_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf("  DA30_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf("  DA30_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf("  DA31_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf("  DA31_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);	
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0008;	
		debugprintf("  DA32_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf("  DA32_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf("  DA33_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf("  DA33_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf("  DA34_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf("  DA34_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf("  DA35_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf("  DA35_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);	
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x0009;	
		debugprintf("  DA36_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf("  DA36_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf("  DA37_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf("  DA37_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf("  DA38_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf("  DA38_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf("  DA39_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf("  DA39_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);	
		debugprintf("---------------------------------------------------------------\r\n");		
		dac_handle->dac_select	=	(unsigned short)0x000a;	
		debugprintf("  DA40_L= %d\r\n",((dac_handle->dac_out0 / 1638) -20)*1000);		
		debugprintf("  DA40_H= %d\r\n",((dac_handle->dac_out1 / 1638) -20)*1000);		
		debugprintf("  DA41_L= %d\r\n",((dac_handle->dac_out2 / 1638) -20)*1000);		
		debugprintf("  DA41_H= %d\r\n",((dac_handle->dac_out3 / 1638) -20)*1000);		
		debugprintf("  DA42_L= %d\r\n",((dac_handle->dac_out4 / 1638) -20)*1000);		
		debugprintf("  DA42_H= %d\r\n",((dac_handle->dac_out5 / 1638) -20)*1000);		
		debugprintf("  DA43_L= %d\r\n",((dac_handle->dac_out6 / 1638) -20)*1000);			
		debugprintf("  DA43_H= %d\r\n",((dac_handle->dac_out7 / 1638) -20)*1000);			
	}
	
	else if(!strncmp(ptr, "ddd", 3))
	{ 
		int i;
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
	}
	else if(!strncmp(ptr, "tiread", 6))
	{	
		debugprintf(" FPGA STATUS= %x\r\n",pg_handle->status);	
		debugprintf(" FPGA CONTROL= %x\r\n",pg_handle->control);			
		debugprintf(" FPGA VOTAL_L= %x\r\n",pg_handle->vtotal_l);			
		debugprintf(" FPGA VOTAL_H= %x\r\n",pg_handle->vtotal_h);			
		debugprintf(" FPGA INVER_L= %x\r\n",pg_handle->inversion_l);			
		debugprintf(" FPGA INVER_M %x\r\n",pg_handle->inversion_m);			
		debugprintf(" FPGA INVER_H %x\r\n",pg_handle->inversion_h);			
		debugprintf(" FPGA SET_L= %x\r\n",pg_handle->set_l);			
		debugprintf(" FPGA SET_M= %x\r\n",pg_handle->set_m);			
		debugprintf(" FPGA SET_H= %x\r\n",pg_handle->set_h);		
		debugprintf(" FPGA SETP1_L_L= %x\r\n",pg_handle->step1_low_l);	
		debugprintf(" FPGA SETP1_L_H= %x\r\n",pg_handle->step1_low_h);			
		debugprintf(" FPGA SETP2_L_L= %x\r\n",pg_handle->step2_high_l);			
		debugprintf(" FPGA SETP2_L_H= %x\r\n",pg_handle->step2_high_h);			
		debugprintf(" FPGA SETP3_L_L= %x\r\n",pg_handle->step3_low_l);			
		debugprintf(" FPGA SETP3_L_H= %x\r\n",pg_handle->step3_low_h);			
		debugprintf(" FPGA SETP4_L_L= %x\r\n",pg_handle->step4_high_l);			
		debugprintf(" FPGA SETP4_L_H= %x\r\n",pg_handle->step4_high_h);			
		debugprintf(" FPGA PER_L= %x\r\n",pg_handle->period_l);			
		debugprintf(" FPGA PER_H= %x\r\n",pg_handle->period_h);	
		debugprintf(" FPGA START_L= %x\r\n",pg_handle->start_l);			
		debugprintf(" FPGA START_H= %x\r\n",pg_handle->start_h);			
		debugprintf(" FPGA END_L= %x\r\n",pg_handle->end_l);			
		debugprintf(" FPGA END_H= %x\r\n",pg_handle->end_h);			
		debugprintf(" FPGA DELAY_L= %x\r\n",pg_handle->output_delay_l);
		debugprintf(" FPGA DELAY_H= %x\r\n",pg_handle->output_delay_h);					
	}
	else if(!strncmp(ptr, "set0", 4))
	{	
		pg_handle->vtotal_l = (unsigned short)400;
		pg_handle->vtotal_h = (unsigned short)0x0001;
	
		pg_handle->inversion_l = (unsigned short)0x0000;
		pg_handle->inversion_m = (unsigned short)0x0000;		
		pg_handle->inversion_h = (unsigned short)0x0000;
		
		pg_handle->start_l = (unsigned short)0;
		pg_handle->start_h = (unsigned short)0;
		
		pg_handle->period_l = (unsigned short)400;
		pg_handle->period_h = (unsigned short)0x0001;
			
		pg_handle->step1_low_l = (unsigned short)200;
		pg_handle->step1_low_h = (unsigned short)0;
		
		pg_handle->step2_high_l = (unsigned short)200;
		pg_handle->step2_high_h = (unsigned short)0;
		
		pg_handle->step3_low_l = (unsigned short)0;
		pg_handle->step3_low_h = (unsigned short)0;
		
		pg_handle->step4_high_l = (unsigned short)0;
		pg_handle->step4_high_h = (unsigned short)0;		
		
		pg_handle->end_l = (unsigned short)400;
		pg_handle->end_h = (unsigned short)0x0001;
		
		pg_handle->set_l = (unsigned short)0xffff;
		pg_handle->set_m = (unsigned short)0xffff;		
		pg_handle->set_h = (unsigned short)0xffff;
		delayms(1);
		pg_handle->set_l = (unsigned short)0x0000;
		pg_handle->set_m = (unsigned short)0x0000;		
		pg_handle->set_h = (unsigned short)0x0000;
	}	

	else if(!strncmp(ptr, "on", 2))
	{
		pg_handle->control = (unsigned short)0x5E;
		delayms(100);
		pg_handle->control = (unsigned short)0x7E;
		debugprintf("Pattern ON\r\n");			
	}
	else if(!strncmp(ptr, "off", 3))
	{
		pg_handle->control = (unsigned short)0x1E;
		delayms(100);
		pg_handle->control = (unsigned short)0x3E;
		debugprintf("Pattern OFF\r\n");			
	}	

	else if(!strncmp(ptr, "fpga", 4))
	{
		debugprintf("FPGA Loading...start\r\n");
		if(fpga_loading()) debugprintf("FPGA Loading...OK\r\n");
		else 					debugprintf("FPGA Loading...Fail\r\n");		
	}		
	else if(!strncmp(ptr, "jed", 3))
	{
		unsigned char data = 0;
		unsigned char data1 = 0;
		unsigned char packet_r[3];		
		
		data = 0x9f;	
		data1 = 0x00;	
		//SPI_SCLK_HIGH;
		SPI_nCS_HIGH;
		SPI_nCS_LOW;	
		byte_data_write(data);
		SPI_SCLK_LOW;
		packet_r[0] = byte_data_read();	
		packet_r[1] = byte_data_read();	
		packet_r[2] = byte_data_read();	
	
		SPI_nCS_HIGH;
		SPI_SCLK_HIGH;
		debugprintf("TP = %x // %x // %x\r\n", packet_r[0], packet_r[1], packet_r[2]);		
	}
	else if(!strncmp(ptr, "manu", 4))
	{
		spi_read(MANUFACTURER_DEVIE_ID, (unsigned int)0x000000,2);
	}
	
	else if(!strncmp(ptr, "te ", 3))
	{	
		char *data_d = NULL;
		char *temp_ptr = NULL;
		unsigned char data = 0;
		
		if((ptr + 3) != NULL)
		{
			temp_ptr = strtok(ptr + 3, ",");
			if(temp_ptr != NULL)
			{
				data = (unsigned int) strtol(temp_ptr, &data_d,16);	
				spi_stat_read(data);
			}
		}
	}
	else if(!strncmp(ptr, "st ", 3))
	{
		char *data_d = NULL;
		char *temp_ptr = NULL;
		unsigned char cmd = 0;
		unsigned char data = 0;
		
		if((ptr + 3) != NULL)
		{
			temp_ptr = strtok(ptr + 3, ",");
			if(temp_ptr != NULL)
			{
				cmd = (unsigned int) strtol(temp_ptr, &data_d,16);	
			}
			temp_ptr = strtok(NULL, ",");
			if(temp_ptr != NULL)
			{
				data = (unsigned int) strtol(temp_ptr, &data_d,16);	
			}			
			spi_stat_write(cmd,data);
		}		
	}
	else if(!strncmp(ptr, "flasher", 6))
	{
		erase_block((unsigned int)FLASH_MEMORY_BLOCK0);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK1);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK2);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK3);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK4);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK5);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK6);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK7);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK8);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK9);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK10);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK11);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK12);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK13);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK14);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK15);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK16);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK17);
		erase_block((unsigned int)FLASH_MEMORY_BLOCK18);	
		debugprintf("FLASH_MEMORY_BLOCK_ERASE_OK\r\n");
	}
	else if(!strncmp(ptr, "wr", 2))
	{
		unsigned char aa[1024] = {0,};
		int i;
		
		for(i = 0 ; i<300; i++)
		{
			aa[i] = 0xaa;
		}

		spi_write(PAGE_PROGREAM,(unsigned int)FLASH_MEMORY_BLOCK0,aa,256);
		debugprintf("WRITE\r\n");
	}

/* 	else if(!strncmp(ptr, "re", 2))
	{
		int i;
		memset(&packet_read_1k, 0, sizeof(packet_read_1k));
		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK0,256);
		memcpy(packet_read_1k,packet_read,256);
		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i,packet_read_1k[i]);
		}
			debugprintf("-----------------------------------------------------------------\r\n");	
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK0+256,256);
		memcpy(packet_read_1k+256,packet_read,256);
		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+256,packet_read_1k[i+256]);
		}
		debugprintf("-----------------------------------------------------------------\r\n");	
					
 		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK0+512,256);
		memcpy(packet_read_1k+512,packet_read,256);
		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+512,packet_read_1k[i+512]);
		}
		debugprintf("-----------------------------------------------------------------\r\n");	
		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK0+768,256);
		memcpy(packet_read_1k+768,packet_read,256);		
 		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+768,packet_read_1k[i+768]);
		}
		debugprintf("-----------------------------------------------------------------\r\n");	
		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK0+1024,256);
		memcpy(packet_read_1k+1024,packet_read,256);		
 		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+1024,packet_read_1k[i+1024]);
		}		
		debugprintf("-----------------------------------------------------------------\r\n");	
	}	
	else if(!strncmp(ptr, "rre", 3))
	{
 		int i;
		memset(&packet_read_1k, 0, sizeof(packet_read_1k));
		
		spi_read(READ_DATA, (unsigned int)(FLASH_MEMORY_BLOCK18+(11*1024)),256);
		memcpy(packet_read_1k,packet_read,256);
		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i,packet_read_1k[i]);
		}
			debugprintf("-----------------------------------------------------------------\r\n");	
		spi_read(READ_DATA, (unsigned int)(FLASH_MEMORY_BLOCK18+(11*1024))+256,256);
		memcpy(packet_read_1k+256,packet_read,256);
		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+256,packet_read_1k[i+256]);
		}
		debugprintf("-----------------------------------------------------------------\r\n");	
					
 		spi_read(READ_DATA, (unsigned int)(FLASH_MEMORY_BLOCK18+(11*1024))+512,256);
		memcpy(packet_read_1k+512,packet_read,256);
		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+512,packet_read_1k[i+512]);
		}
		debugprintf("-----------------------------------------------------------------\r\n");	
		
		spi_read(READ_DATA, (unsigned int)(FLASH_MEMORY_BLOCK18+(11*1024))+768,256);
		memcpy(packet_read_1k+768,packet_read,256);		
 		for(i=0 ; i <256 ; i++)
		{
			debugprintf("READ(%d) = %x\r\n", i+768,packet_read_1k[i+768]);
		}
		debugprintf("-----------------------------------------------------------------\r\n");		 		
	} */
	else if(!strncmp(ptr, "rrr", 3))
	{
		int i;
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK0,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ0(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");			
		spi_read(READ_DATA, (unsigned int)0x00FFF0,256);
		for(i=0 ; i <20 ; i++)
		{
			debugprintf("READ00(%d) = %x\r\n", i,packet_read[i]);
		}		
		debugprintf("-----------------------------------------------------------------\r\n");	
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK1,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ1(%d) = %x\r\n", i,packet_read[i]);
		}		
		debugprintf("-----------------------------------------------------------------\r\n");	
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK2,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ2(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK3,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ3(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK4,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ4(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK5,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ5(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK6,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ6(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK7,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ7(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK8,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ8(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK9,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ9(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK10,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ10(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK11,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ11(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK12,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ12(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK13,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ13(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK14,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ14(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK15,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ15(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ16(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK17,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ17(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		spi_read(READ_DATA, (unsigned int)FLASH_MEMORY_BLOCK18,256);
		for(i=0 ; i <10 ; i++)
		{
			debugprintf("READ18(%d) = %x\r\n", i,packet_read[i]);
		}	
		debugprintf("-----------------------------------------------------------------\r\n");		
		
		
	}
	else if(!strncmp(ptr, "model print", 11))
	{
		MODEL_PRINT();
	}
	else debugprintf("Command Not Found\r\n");
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

 void com_task(unsigned char *data, unsigned short length)
{
	unsigned short flag;
	int i;
	int data_len;
	unsigned short cal_checksum = 0;
	PROTOCOL_STRUCT protocol;
	
	if((length < 6) || (data == NULL)){
		debugprintf("Invalid rs232 recv data\r\n");
		return;
	}
	
	protocol.cmd = (unsigned short *)&data[0];
	protocol.length = (unsigned short *)&data[2];
	data_len = (int)(*protocol.length);
	protocol.data = (unsigned char *)&data[4];
	//if(data_len > 0) protocol.data = (unsigned char *)&data[4];
	//else protocol.data = NULL;
	protocol.checksum = (unsigned short )((data[4+data_len+1] << 8) | data[4+data_len]);	
	for(i=0; i<length-2; i++) cal_checksum += (unsigned short)(data[i] & 0xff);

  	if(cal_checksum != (protocol.checksum)){
		debugprintf("rs232 checksum error\r\n");
		return;
	} 
	if(((*protocol.cmd) == COMMAND_PAT_SET) && (data_len == 1356)){
		memset(&ensis_model_gp, 0, sizeof(ensis_model_gp));
		//memcpy(&ensis_model_gp.pat_conf[0], protocol.data, (*protocol.length < sizeof(ensis_model_gp.pat_conf)) ? *protocol.length : sizeof(ensis_model_gp.pat_conf));
		memmove(&ensis_model_gp.pat_conf[0], protocol.data, (*protocol.length < sizeof(ensis_model_gp.pat_conf)) ? *protocol.length : sizeof(ensis_model_gp.pat_conf));
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
		com_ack(COMMAND_PAT_SET_ACK);
		debugprintf("PATTERN CHANGE\r\n");
	}
	else if(((*protocol.cmd) == COMMAND_PAT_ONOFF) && (data_len == 1)){
 		if(*protocol.data == 1){
			flag = pg_handle->control | (PG_DAC_LOAD_OPERATION|PG_SIGNAL_INVERSION_CHANGE|PG_TIMING_CHANGE|PG_ANALOG_MUX_ENABLE);
			if(!SET_PG_CONTROL(&flag))
			{
				debugprintf("SET PG CONTROL Fail\r\n");
				return;
			}
			delayms(1);
			com_ack(COMMAND_PAT_ONOFF_ACK);
			debugprintf("Pattern ON\r\n");
		}
		else{
			flag = pg_handle->control & (~PG_ANALOG_MUX_ENABLE);
			if(!SET_PG_CONTROL(&flag))
			{
				debugprintf("SET PG CONTROL Fail\r\n");
				return;
			}
			delayms(1);
			com_ack(COMMAND_PAT_ONOFF_ACK);
			debugprintf("Pattern OFF\r\n");
		} 
	}
	
	else if(((*protocol.cmd) == COMMAND_STATUS_REQ) && (data_len == 0)){
		com_ack(COMMAND_STATUS_REQ_ACK);
	}
	else if(((*protocol.cmd) == COMMAND_DYNAMIC_SIGNAL) && (data_len == 5)){
		DYNAMIC_SIG dy_sig;
		
		memcpy(&dy_sig, protocol.data, 5);
		ensis_model_gp.pat_conf[0].sig_conf[dy_sig.channel_number].volt_high = dy_sig.volt_high;
		ensis_model_gp.pat_conf[0].sig_conf[dy_sig.channel_number].volt_low = dy_sig.volt_low;
			
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
		com_ack(COMMAND_DYNAMIC_SIGNAL_ACK);
		debugprintf("Dynamic Signal!\n");
	}
	else if(((*protocol.cmd) == COMMAND_VERSION_REQ) && (data_len == 0)){
		com_ack(COMMAND_VERSION_REQ_ACK);
	}
	else{}
}

void com_ack(unsigned short command)
{
	if(command == COMMAND_PAT_SET_ACK){
		com_ack_send(command, NULL, 0);
	}
	else if(command == COMMAND_PAT_ONOFF_ACK){
		com_ack_send(command, NULL, 0);
	}
	else if(command == COMMAND_STATUS_REQ_ACK){
		COM_ACK_VOLT voltage;
		VOLT_ACK_MAPPING(&voltage);
		//com_ack_send(command, (unsigned char *)&voltage, 92);
		com_ack_send_elp_protocol(command, (unsigned char *)&voltage, 92);
	}
	else if(command == COMMAND_DYNAMIC_SIGNAL_ACK){
		com_ack_send(command, NULL, 0);
	}
	else if(command == COMMAND_VERSION_REQ_ACK){
		unsigned char version[2];
		
		version[0] = FIRMWARE_VERSION;
		version[1] = 10;//수정필요
		com_ack_send(command, version, 2);
	}
	else{}
}

static void com_ack_send(unsigned short cmd, unsigned char *data, int length)
{
	int i;
	int index = 0;
	unsigned char *tx_data;
	unsigned short tx_check_sum = 0;
	
	tx_data = (unsigned char *)malloc(length+4+2+2+2+4);
	if(tx_data == NULL){
		debugprintf("tx_data malloc fail\r\n");
		return;
	}
	
	tx_data[index++] = 0x37;tx_data[index++] = 0xa4;tx_data[index++] = 0xc2;tx_data[index++] = 0x95;
	tx_data[index++] = (unsigned char)(cmd >> 0);tx_data[index++] = (unsigned char)(cmd >> 8);
	tx_data[index++] = (unsigned char)(length >> 0);tx_data[index++] = (unsigned char)(length >> 8);
	if(length > 0){
        memcpy(&tx_data[index], data, length);
        index += length;
    }
	for(i=4; i<index; i++) tx_check_sum += (unsigned short)(tx_data[i] & 0xff);
	tx_data[index++] = (unsigned char)(tx_check_sum >> 0);tx_data[index++] = (unsigned char)(tx_check_sum >> 8);
    tx_data[index++] = 0x59;tx_data[index++] = 0x2c;tx_data[index++] = 0x4a;tx_data[index++] = 0x0d;
	
	for(i=0; i<index; i++) UartPutCh(COM_UART, *((char *)(tx_data+i)));
	
	free(tx_data);
}

static void VOLT_ACK_MAPPING(COM_ACK_VOLT *voltage)
{
	//수정필요
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
   voltage->voltage[33] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[34] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[35] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[36] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[37] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[38] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[39] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[40] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[41] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[42] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[43] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[44] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
   voltage->voltage[45] = adc_sensing_value.value[25];                                                                                      //VOLT_DIGITAL24
}

static void com_ack_send_elp_protocol(unsigned short cmd, unsigned char *data, int length)
{
	int i;
	int index = 0;
	unsigned char *tx_data;
	unsigned char tx_checksum = 0;
	
	tx_data = (unsigned char *)malloc(1 + 1 + 1 +  1 + 92 + 1 + 1);	//STX(1), STATUS(1), TYPE(1), LENGTH(1), User Data(92), Checksum(1), ETX(1)
	if(tx_data == NULL){
		debugprintf("volt req tx_data malloc fail\r\n");
		return;
	}
	tx_data[index++] = 0x02; //STX
	tx_data[index++] = 0xEE; //STATUS
	tx_data[index++] = 0x80; //TYPE
	tx_data[index++] = (unsigned char)length; //LENGTH
	if(length > 0){
		memcpy(&tx_data[index], data, length);
		index += length;
	}
	for(i=1; i<(length + 4); i++) tx_checksum ^= tx_data[i];
	tx_data[index++] = tx_checksum; //CheckSum
	tx_data[index++] = 0x03; //ETX
	
	for(i=0; i<index; i++) UartPutCh(COM_UART, *((char *)(tx_data+i)));
	
	free(tx_data);
}

void spi_stat_read(unsigned char cmd)
{
	unsigned char stat_num = 0;
	unsigned char	stat_read = 0;
	if(cmd == 1)			stat_num = READ_STAT_REGISTER_1;
	else if(cmd ==2)	stat_num = READ_STAT_REGISTER_2;
	else if(cmd ==3)	stat_num = READ_STAT_REGISTER_3;	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	byte_data_write(stat_num);
	SPI_SCLK_LOW;
	stat_read = byte_data_read();	
	SPI_nCS_HIGH;
	SPI_SCLK_HIGH;
	debugprintf("STAT%d = %x\r\n",cmd, stat_read );
}
void spi_stat_write(unsigned char cmd, unsigned char data)
{
	unsigned char stat_num = 0;
	if(cmd == 1)			stat_num = WRITE_STAT_REGISTER_1;
	else if(cmd ==2)	stat_num = WRITE_STAT_REGISTER_2;
	else if(cmd ==3)	stat_num = WRITE_STAT_REGISTER_3;
	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;

	SPI_nCS_LOW;
	byte_data_write(STAT_WRITE_ENABLE_F);
	SPI_nCS_HIGH;
	SPI_SCLK_HIGH;	
	SPI_nCS_LOW;
	byte_data_write(stat_num);
	byte_data_write(data);	
	SPI_nCS_HIGH;
	SPI_SCLK_HIGH;		
}
void spi_write(unsigned char cmd,unsigned int addr, unsigned char  *data,int len)
{
  	unsigned int  i;	
	unsigned char packet[1+3+len];
		
	packet[0]=cmd;
	packet[1]=(addr>>16)&0xFF;
	packet[2]=(addr>>8)&0xFF;
	packet[3]=addr&0xFF;	
	while (w25q32.Lock == 1)
	//delayms(1);	
	w25q32.Lock = 1;	
	memcpy(packet+4,data,len);
	wait_for_write_end();
	write_enable();		
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;

	for(i = 0 ; i<len+4 ; i++)
	{
		byte_data_write(packet[i]);
	}
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	wait_for_write_end();
	w25q32.Lock = 0;
	i=0;
}
//unsigned char  *spi_read(unsigned char cmd,unsigned int addr,int len)
void  spi_read(unsigned char cmd,unsigned int addr,int len)
{
	unsigned int  i,j;	
	unsigned char packet_w[5];
		
	packet_w[0]=cmd;
	packet_w[1]=(addr>>16)&0xFF;
	packet_w[2]=(addr>>8)&0xFF;
	packet_w[3]=addr&0xFF;	
	packet_w[4]=0x00;
	
	memset(&packet_read, 0, sizeof(packet_read));
	
	while (w25q32.Lock == 1)
	delayms(1);	
	w25q32.Lock = 1;	
	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
 	if(cmd == 0x0B)
	{
		for(i = 0 ; i < 5 ; i++)
		{
			byte_data_write(packet_w[i]);	
		}
	}
	else
	{
		for(i = 0 ; i < 4 ; i++)
		{
			byte_data_write(packet_w[i]);	
		}	
	} 
	SPI_SCLK_LOW;
 	for(j=0;j<len;j++)
	{		
		packet_read[j] = byte_data_read();
	} 
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	i = 0;
	j = 0;
	w25q32.Lock = 0;
}

void byte_data_write(unsigned char data)
{
	unsigned int  i;	
	//debugprintf("WRITE = %x\r\n", data);
	//delayms(1);
	SPI_SCLK_HIGH;
	for(i = 0x80 ; i > 0 ; i>>=1)
	{
		//delayms(1);
		SPI_SCLK_LOW;
		//delayms(1);
		if(data & i)	SPI_MOSI_HIGH;
		else				SPI_MOSI_LOW;	
		//delayms(1);	
		SPI_SCLK_HIGH;
	}
}

unsigned char byte_data_read()
{
	unsigned int  i;	
	unsigned char rev = 0;
	unsigned char rev_d = 0;	
	
	for(i = 0 ; i<8 ; i++)
	{
		//delayms(1);	
		SPI_SCLK_HIGH;
		//delayms(1);	
		rev = ((*R_P5iLEV >> 4) & 0x01);
		//delayms(1);	
		SPI_SCLK_LOW;
		rev_d = (rev_d<<0x01) | rev;
		//debugprintf("REV = %x\r\n",rev);
	}
	//debugprintf("STAT  = %x \r\n", rev_d);
	return rev_d;
}

void wait_for_write_end()
{
	//delayms(1);
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	//delayms(1);
	SPI_nCS_LOW;
	byte_data_write(READ_STAT_REGISTER_1);
	SPI_SCLK_LOW;
	do
	{
		w25q32.StatusRegister1 = byte_data_read();
		//delayms(1);
	}while	((w25q32.StatusRegister1 & 0x01)	==	0x01);
	SPI_SCLK_HIGH;
	//delayms(1);
	SPI_nCS_HIGH;	
} 
void write_enable()
{
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	//delayms(1);
	SPI_nCS_LOW;
	//delayms(1);
	byte_data_write(WRITE_ENABLE_F);
	SPI_SCLK_HIGH;
	//delayms(1);
	SPI_nCS_HIGH;
	//delayms(1);
}
void write_disable()
{
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	delayms(1);
	SPI_nCS_LOW;
	delayms(1);
	byte_data_write(WRITE_DISABLE_F);
	SPI_SCLK_HIGH;	
	delayms(1);
	SPI_nCS_HIGH;
	delayms(1);
}
void flash_init()
{
/* 	SPI_nRESET_HIGH;
	SPI_nCS_HIGH;
	SPI_MOSI_HIGH;
	SPI_SCLK_HIGH;	
	SPI_nWP_HIGH; */
	unsigned char stat_num = 0;
	int i;
	w25q32.Lock = 0;
	w25q32.BlockCount = 64;	
	w25q32.PageSize = 256;
	w25q32.SectorSize = 0x1000;
	w25q32.SectorCount = w25q32.BlockCount * 16;
	w25q32.PageCount = (w25q32.SectorCount * w25q32.SectorSize) / w25q32.PageSize;
	w25q32.BlockSize = w25q32.SectorSize * 16;
	w25q32.CapacityInKiloByte = (w25q32.SectorCount * w25q32.SectorSize) / 1024;	
	
	spi_stat_write(1,0x00);
	spi_stat_write(2,0x00);
	spi_stat_write(3,0x00);
	
	for(i=1 ; i<4 ; i++)
	{
		if(i == 1)			stat_num = READ_STAT_REGISTER_1;
		else if(i ==2)	stat_num = READ_STAT_REGISTER_2;
		else if(i ==3)	stat_num = READ_STAT_REGISTER_3;	
		SPI_SCLK_HIGH;
		SPI_nCS_HIGH;
		SPI_nCS_LOW;
		byte_data_write(stat_num);
		SPI_SCLK_LOW;
		if(stat_num == READ_STAT_REGISTER_1)	w25q32.StatusRegister1 = byte_data_read();	
		else if(stat_num == READ_STAT_REGISTER_2)	w25q32.StatusRegister2 = byte_data_read();	
		else if(stat_num == READ_STAT_REGISTER_3)	w25q32.StatusRegister3 = byte_data_read();	
		SPI_nCS_HIGH;
		SPI_SCLK_HIGH;	
	}
	debugprintf("w25qxx Page Size: %d Bytes\r\n", w25q32.PageSize);
	debugprintf("w25qxx Page Count: %d\r\n", w25q32.PageCount);
	debugprintf("w25qxx Sector Size: %d Bytes\r\n", w25q32.SectorSize);
	debugprintf("w25qxx Sector Count: %d\r\n", w25q32.SectorCount);
	debugprintf("w25qxx Block Size: %d Bytes\r\n", w25q32.BlockSize);
	debugprintf("w25qxx Block Count: %d\r\n", w25q32.BlockCount);
	debugprintf("w25qxx Capacity: %d KiloBytes\r\n", w25q32.CapacityInKiloByte);
	debugprintf("w25qxx Init Done\r\n");	
	debugprintf("StatusRegister1 = %x / StatusRegister2 = %x / StatusRegister3 = %x\r\n", w25q32.StatusRegister1,w25q32.StatusRegister2,w25q32.StatusRegister3);
} 
void erase_block(unsigned int blockaddr)
{
	unsigned char packet[4] = {0,};
	int i;
	packet[0] = BLOCK_ERASE_64K;
	while (w25q32.Lock == 1)
	delayms(1);	
	w25q32.Lock = 1;
	//debugprintf("EraseBlock %d Begin...\r\n", blockaddr);
	delayms(100);
	wait_for_write_end();
	//blockaddr = blockaddr * w25q32.SectorSize * 16;
	write_enable();
	wait_for_write_end();	
	packet[1]=(blockaddr>>16)&0xFF;
	packet[2]=(blockaddr>>8)&0xFF;
	packet[3]=blockaddr&0xFF;	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
	delayms(1);
 	for(i = 0 ; i < 4 ; i++)
	{
		byte_data_write(packet[i]);	
	}   
	
	SPI_SCLK_HIGH;
	delayms(1);
	SPI_nCS_HIGH;	
	wait_for_write_end();
	//debugprintf("w25q32 EraseBlock done\r\n");
	delayms(100);
	w25q32.Lock = 0;
}

void write_byte(char buffer, int write_addr)
{
	while (w25q32.Lock == 1)
	delayms(1);	
	w25q32.Lock = 1;	
	printf("w25q32 WriteByte 0x%02X at address %d begin...", buffer, write_addr);	
	
	wait_for_write_end();
	write_enable();	
}

unsigned char byte_fpga_read(unsigned char cmd,unsigned int addr)
{
	unsigned int  i;	
	unsigned char rev = 0;
	unsigned char rev_d = 0;	
	unsigned char packet_w[5];
		
	packet_w[0]=cmd;
	packet_w[1]=(addr>>16)&0xFF;
	packet_w[2]=(addr>>8)&0xFF;
	packet_w[3]=addr&0xFF;	
	packet_w[4]=0x00;

	delayms(1);	
	
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	SPI_nCS_LOW;
 	if(cmd == 0x0B)
	{
		for(i = 0 ; i < 5 ; i++)
		{
			byte_data_write(packet_w[i]);	
		}
	}
	else
	{
		for(i = 0 ; i < 4 ; i++)
		{
			byte_data_write(packet_w[i]);	
		}	
	} 
	SPI_SCLK_LOW;
	for(i = 0 ; i<8 ; i++)
	{
		//delayms(1);	
		SPI_SCLK_HIGH;
		//delayms(1);	
		rev = ((*R_P5iLEV >> 4) & 0x01);
		//delayms(1);	
		SPI_SCLK_LOW;
		rev_d = (rev_d<<0x01) | rev;
	}
	//debugprintf("STAT  = %x \r\n", rev_d);
	return rev_d;
	SPI_SCLK_HIGH;
	SPI_nCS_HIGH;
	i = 0;
}


