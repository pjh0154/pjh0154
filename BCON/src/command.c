/*
 * command.c
 *
 *  Created on: 2019. 4. 25.
 *      Author: ghkim
 */
#include "command.h"

#if defined(DEBUG_UART_MODE_POLLING) || defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA)
static char debug_buffer[DEBUG_BUFFER_SIZE] = {0};
uint8_t debug_put = 0;
static uint8_t debug_put_cnt = 0;
static void debug(char *ptr);
#endif
#if defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA) || defined(COM_UART_MODE_POLLING)
#define STX ((uint64_t)0x37a4c29537a4c295)
#define ETX ((uint64_t)0x592c4a0d592c4a0d)
uint8_t com_buffer_intterrupt[COM_BUFFER_SIZE] = {0};
int com_buffer_intterrupt_cnt = 0;
int com_buffer_cnt = 0;
char com_buffer[COM_BUFFER_SIZE] = {0};
char com_buffer1[128] = {0};
uint64_t com_put_check = 0;
uint8_t com_put = 0;
extern uint8_t aprf;
float com_put_2=0;
float test_value_2 = 0;
double adc_value1;
static uint16_t com_put_cnt = 0;
static uint8_t com_packet_state = 0;
static void com_task(char *ptr);
//void printf_bin(long long data);
#endif

extern UART_HandleTypeDef DEBUG_UART_PORT;
extern UART_HandleTypeDef COM_UART_PORT;
extern IWDG_HandleTypeDef hiwdg;

uint8_t cprf=0;
uint8_t dprf=0;
float keithley_data;
#define Cprintf if(cprf) printf
#define Dprintf if(dprf) printf
extern uint8_t bprf;
static void com_task_ack(uint16_t cmd);
unsigned char bcon_output_on_off_select[60];
//Protocol Definition
/*#define BRODCAST_ID						0x00aa
#define MODEL_CONFIG					0x0001
#define MODEL_CONFIG_LEN				182
#define BIST_CONFIG						0x0003
#define BIST_CONFIG_LEN					480
#define BIST_START						0x0005
#define BIST_START_LEN					0
#define BIST_STATUS_REQ					0x0007
#define BIST_STATUS_REQ_LEN				0
#define BIST_STATUS_REQ_ACK				0x0008
#define BIST_STATUS_REQ_ACK_LEN			30
#define BCON_OUTPUT_ON_OFF				0x0009
#define BCON_OUTPUT_ON_OFF_LEN			1
#define FW_VERSION_REQ					0x000B
#define FW_VERSION_REQ_LEN				0
#define FW_VERSION_REQ_ACK				0x000C
#define FW_VERSION_REQ_ACK_LEN			1
#define BCON_SELECT_OUTPUT_ON_OFF		0x000D
#define BCON_SELECT_OUTPUT_ON_OFF_LEN	60*/

#define BRODCAST_ID						0x00aa
#define MODEL_CONFIG					0x0001
#define MODEL_CONFIG_LEN				482
#define BIST_CONFIG						0x0003
#define BIST_CONFIG_LEN					384
#define BIST_START						0x0005
#define BIST_START_LEN					0
#define BIST_STATUS_REQ					0x0007
#define BIST_STATUS_REQ_LEN				0
#define BIST_STATUS_REQ_ACK				0x0008
#define BIST_STATUS_REQ_ACK_LEN			42
#define BCON_OUTPUT_ON_OFF				0x0009
#define BCON_OUTPUT_ON_OFF_LEN			1
#define FW_VERSION_REQ					0x000B
#define FW_VERSION_REQ_LEN				0
#define FW_VERSION_REQ_ACK				0x000C
#define FW_VERSION_REQ_ACK_LEN			1
#define BCON_SELECT_OUTPUT_ON_OFF		0x000D
#define BCON_SELECT_OUTPUT_ON_OFF_LEN	48
#define BCON_FWDOWNLOAD 				0x0011
#define BCON_FWDOWNLOAD_LEN				1032
#define BIST_OFFSET						0x000E
#define BIST_OFFSET_LEN					8
#define FW_PACKET_SIZE					1024
//KEITHLEY TEST
#define KEITHLEY_READ_REQ				0x000F
#define KEITHLEY_READ_REQ_LEN			0
#define KEITHLEY_READ_REQ_ACK			0x0010
#define KEITHLEY_READ_REQ_ACK_LEN		4
#define FLASH_ERASE_SIZE				0x4000
#define KEITHLEY_READ_START				0x0013
#define KEITHLEY_READ_START_LEN			0
#define	KEITHLEY_READ_END				0x0015
#define KEITHLEY_READ_END_LEN			0

#define	OSG_L_MODE						0x1000
#define	OSG_R_MODE						0x2000
#define	DATA_SSD_MODE					0x3000
#define	DATA_NEW_MODE					0x4000
#define	DATA_77_MODE					0x5000
#define	DATA_OSG_MODE					0x6000
#define MODE_LEN						0

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	#if defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA) || defined(DEBUG_UART_MODE_POLLING)
	if(huart->Instance == DEBUG_UART_PORT.Instance)
	{
		if((debug_put == '\r') || (debug_put == '\n'))
		{
			printf("\r\n");
			if(debug_put_cnt != 0)
			{
				debug(debug_buffer);
				debug_put_cnt = 0;
				memset(debug_buffer, 0, sizeof(debug_buffer));
			}
			printf(PROMPT);
		}
		else if(debug_put == 0x08)
		{
			if(debug_put_cnt > 0)
			{
				uint8_t temp[3] = {0x08, 0x20, 0x08};
				HAL_UART_Transmit(&DEBUG_UART_PORT, temp, 3, 100);
				debug_buffer[--debug_put_cnt] = '\0';
			}
		}
		else
		{
			if(debug_put_cnt < DEBUG_BUFFER_SIZE)
			{
				debug_buffer[debug_put_cnt++] = debug_put;
				HAL_UART_Transmit(&DEBUG_UART_PORT, &debug_put, 1, 100);
			}

		}
		#ifdef DEBUG_UART_MODE_INTERRUPT
		HAL_UART_Receive_IT(&DEBUG_UART_PORT, &debug_put, 1);
		#endif
	}
	#endif

	#if defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA) || defined(COM_UART_MODE_POLLING)
	if(huart->Instance == COM_UART_PORT.Instance)
	{
		Dprintf("%02x", com_put);
		com_put_check = (com_put_check << 8) + (com_put&0xff);
		if(com_put_check == STX)
		{
			com_packet_state = 1;
			com_put_cnt = 0;
			memset(com_buffer, 0, sizeof(com_buffer));
			memset(&packet, 0, sizeof(packet));
		}
		else if((com_put_check == ETX) && (com_packet_state == 1))
		{
			com_packet_state = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt--] = 0;
			com_buffer[com_put_cnt] = 0;
			com_task(com_buffer);
		}
		else
		{
			if(com_packet_state == 1) com_buffer[com_put_cnt++] = com_put;
			if(com_put_cnt >= COM_BUFFER_SIZE) com_packet_state = 0;
		}
	}
	#endif
}

//extern IWDG_HandleTypeDef hiwdg;
#if defined(DEBUG_UART_MODE_POLLING) || defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA)
static void debug(char *ptr)
{
	if(!strncmp(ptr, "help", 4))
	{
		//int i=0;
		//for(i = 0 ; i< 50 ; i++)
		//{
			printf("============== Command List ==============\r\n");
			printf("Software Reset = reset\r\n");
			printf("BCON Operation Print On/Off Toggle = bprf\r\n");
			printf("RS485 Print On/Off Toggle = cprf\r\n");
			printf("BCON Model Information Print = model\r\n");
			printf("BIST RESULT Print = bistvalue\r\n");
			printf("BIST RESULT COM TX = bistack\r\n");
			printf("BIST Start = biststart\r\n");

			printf("BCON MODE Select = mode XXXXX(DECIMAL VALUE)\r\n");
			//printf("<< NONE(0)/DATA(1)/OSG_ACT(2)/OSG_VT(3),FORWARD(0)/REVERSE(1),3D(0)/6D(1),BOTTOM(0)/TOP(1), LED_OFF(0)/LED_ON(1) >>\r\n");
			printf("<< NONE(0)/O_DATA(1)/S_DATA(2)/O_12K(3)/O_8K(4)/O_6K(5),FORWARD(0)/REVERSE(1), LED_OFF(0)/LED_ON(1) >>\r\n");
			printf("BCON OUTPUT Enable Setting = out XXXXXXXX(HEX Value)\r\n");
			printf("BCON OUTPUT ON = out on\r\n");
			printf("BCON OUTPUT OFF = out off\r\n");
			printf("BCON BIST MODE USE(1)/NOT USE(0) Setting = bistuse XXXXXXXX(HEX Value)\r\n");
			printf("BCON BIST MODE ADC(0)/DAC(1) Setting = bistadc XXXXXXXX(HEX Value)\r\n");
			printf("BCON BIST OFFSET VALUE Setting = bistoffset XXXX(Decimal Value)\r\n");
			printf("BCON BIST OFFSET VALUE Setting = bistoffset XXXX(Decimal Value)\r\n");
			printf("BIST CAL DATA INIT = bistcalinit\r\n");
			printf("BIST 0V CAL = bistcal0v XXXX(Decimal Value)\r\n");
			printf("BIST 15V CAL = bistcal5v XXXX(Decimal Value)\r\n");
			printf("BIST CAL DATA calculation = bistcal\r\n");
			printf("BIST OFFSET Setting = bistoffset\r\n");
			printf("BIST CAL DATA SAVE = bistcalsave\r\n");
			printf("BIST CAL DATA PRINT = bistprint\r\n");
			printf("BCON OSG_L MODE ADC TEST = testosgl\r\n");
			printf("BCON OSG_R MODE ADC TEST = testosgr\r\n");
			printf("BCON DATA_NEW MODE ADC TEST = testdata\r\n");
			printf("BCON DATA_SSD MODE ADC TEST = testssddata\r\n");
			printf("BCON DATA_77 MODE ADC TEST = test77data\r\n");
			printf("BCON DATA_OSG MODE ADC TEST = testosgdata\r\n");
			printf("BCON OSG12CLK_L MODE CHANGE = osg12l\r\n");
			printf("BCON OSG12CLK_R MODE CHANGE = osg12r\r\n");
			printf("BCON DATA_NEW   MODE CHANGE = datanew\r\n");
			printf("BCON DATA_SSD   MODE CHANGE = datassd\r\n");
			printf("BCON DATA_77    MODE CHANGE = data77\r\n");
			printf("BCON DATA_OSG   MODE CHANGE = dataosg\r\n");
			printf("JIG or NORMAL   MODE CHANGE = jig\r\n");
			printf("============== Command List ==============\r\n");

			//HAL_IWDG_Refresh(&hiwdg);
		//}

	}
	else if(!strncmp(ptr, "reset", 5))
	{
		NVIC_SystemReset();
	}
	else if(!strncmp(ptr, "bprf", 4))
	{
		if(bprf == 0)
		{
			bprf = 1;
			printf("BCON Operation Print ON\r\n");
		}
		else
		{
			bprf = 0;
			printf("BCON Operation Print OFF\r\n");
		}
	}
	else if(!strncmp(ptr, "cprf", 4))
	{
		if(cprf == 0)
		{
			cprf = 1;
			printf("RS485 Print ON\r\n");
		}
		else
		{
			cprf = 0;
			printf("RS485 Print OFF\r\n");
		}
	}
	else if(!strncmp(ptr, "dprf", 4))
	{
		if(dprf == 0)
		{
			dprf = 1;
			printf("RS485 DATA ON\r\n");
		}
		else
		{
			dprf = 0;
			printf("RS485 DATA OFF\r\n");
		}
	}
	else if(!strncmp(ptr, "aprf", 4))
	{
		if(aprf == 0)
		{
			aprf = 1;
			printf("ADC DATA ON\r\n");
		}
		else
		{
			aprf = 0;
			printf("ADC DATA OFF\r\n");
		}
	}
	else if(!strncmp(ptr, "fwtest", 6))
	{
		uint8_t str[12] = "fwtest121";
		if(FLASH_If_Erase(FW_UPDATA_ADDRESS, (uint32_t)sizeof(str)) == HAL_OK)
		{
			if(FLASH_If_Write(FW_UPDATA_ADDRESS, (uint32_t *)str, 3) == HAL_OK)
			{
				Cprintf("Number 0 FLASH WRITE SUCCESS\r\n");
			}
			else Cprintf("Number 0 FLASH WRITE FAIL\r\n");
		}
		else Cprintf("FLASH ERASE FAIL\r\n");
	}
	else if(!strncmp(ptr, "fw0", 3))
	{
		printf("TP = %d / %d\r\n", fw_flag.fw_flag, fw_flag.number);
	}
	else if(!strncmp(ptr, "fw1", 3))
	{
		uint8_t prt[12] = {0,};
		memset(&prt, 0, sizeof(prt));
		memcpy(&prt, (uint32_t *)FW_UPDATA_ADDRESS, sizeof(prt));
		printf("TP1 = %s\r\n", prt);

	}
	else if(!strncmp(ptr, "fw2", 3))
	{
		fw_flag.fw_flag = 1;
		//uint32_t str = 1;
		if(FLASH_If_Write(FW_UPDATA_ADDRESS+12, (uint32_t *)&fw_flag, (sizeof(fw_flag))/4) == HAL_OK)
		{
			Cprintf("Number 0 FLASH WRITE SUCCESS\r\n");
		}
		else Cprintf("Number 0 FLASH WRITE FAIL\r\n");
	}
	else if(!strncmp(ptr, "fw3", 3))
	{
/*		fw_flag.fw_flag = 1;
		//uint32_t str = 1;
		if(FLASH_If_Erase(FW_UPDATA_FLAG, (uint32_t)sizeof(fw_flag)) == HAL_OK)
		{
			if(FLASH_If_Write(FW_UPDATA_FLAG, (uint32_t *)&fw_flag, (sizeof(fw_flag))/4) == HAL_OK)
			{
				Cprintf("Number 0 FLASH WRITE SUCCESS\r\n");
			}
			else Cprintf("Number 0 FLASH WRITE FAIL\r\n");
		}*/

		fw_flag.fw_flag = 1;
		fw_flag.number = 64;
		if(FLASH_If_Erase(FW_UPDATA_FLAG, (uint32_t)sizeof(fw_flag)) == HAL_OK)
		{
			if(FLASH_If_Write(FW_UPDATA_FLAG, (uint32_t *)&fw_flag, (sizeof(fw_flag))/4) == HAL_OK)
			{
				Cprintf("FW FLAG FLASH WRITE SUCCESS\r\n");
				fw_down_count = 0;
				NVIC_SystemReset();

			}
			else Cprintf("FW FLAG FLASH WRITE FAIL\r\n");
		}
	}
	else if(!strncmp(ptr, "model", 5))
	{
/*		printf("FW VERSION = %d\r\n",FW_VERSION);
		printf("BCON BASE ID = 0x%02x\r\n",BCON_BASE_ID);
		printf("BCON ID = 0x%02x\r\n",BCON_ID);
		printf("BCON MODE NONE(0)/DATA(1)/OSG_ACT(2)/OSG_VT(3) = %d\r\n",(unsigned char)bcon_mode_osg_data);
		printf("BCON MODE FORWARD(0)/REVERSE(1) = %d\r\n",(unsigned char)bcon_mode_direction);
		//printf("BCON MODE 3D(0)/6D(1) = %d\r\n",(unsigned char)bcon_mode_3d_6d);
		//printf("BCON MODE BOTTOM(0)/TOP(1) = %d\r\n",(unsigned char)bcon_mode_el_swap);
		printf("BCON MODE OUTPUT USE CHANNEL ENABLE(HIGH)/DISABLE(LOW) = 0x%llx\r\n",(long long)bcon_output_enable_data);
		printf("BIST MODE USE(HIGH)/NOT USE(LOW) = 0x%llx\r\n",(long long)bist_mode_use);
		printf("BIST MODE ADC(LOW)/DAC(HIGH) = 0x%llx\r\n",(long long)bist_mode_adc_dac);
		printf("BIST ADC OFFSET VALUE = %d\r\n",bist_offset);*/
		printf("FW VERSION =                                                                                                 %d\r\n",FW_VERSION);
		printf("BCON BASE ID =                                                                                               0x%02x\r\n",BCON_BASE_ID);
		printf("BCON ID =                                                                                                    0x%02x\r\n",BCON_ID);
		printf("BCON MODE NONE(0)/O_DATA(1)/S_DATA(2)/O_12K(3)/O_8K(4)/O_6K(5),FORWARD(0)/REVERSE(1) =                       %d\r\n",(unsigned char)bcon_mode_osg_data);
		printf("BCON MODE FORWARD(0)/REVERSE(1) =                                                                            %d\r\n",(unsigned char)bcon_mode_direction);
		printf("RECV BCON MODE LED_OFF(0)/LED_ON(1) =                                                                        %d\r\n",(unsigned char)bcon_led_on_off);
		printf("BCON MODE OUTPUT USE CHANNEL ENABLE(HIGH)/DISABLE(LOW) =                                                     0x%llx\r\n",(long long)bcon_output_enable_data);
		printf("BIST MODE USE(HIGH)/NOT USE(LOW) =                                                                           0x%llx\r\n",(long long)bist_mode_use);
		printf("BIST ADC OFFSET VALUE =                                                                                      %d\r\n",bist_offset);
		printf("BIST ZERO OFFSET VALUE =                                                                                     %d\r\n",z_offset);
	}
	else if(!strncmp(ptr, "biststart", 9))
	{
		//bcon_mode_osg_data = OSG_12CLK;
		//bcon_mode_direction = FORWARD;
		//bcon_led_on_off = LED_ON;
		//bist_mode_use = (long long)0x3ffffffffff;
		//bcon_mode_output_enable_mask = (long long)0x3ffffffffff;
		//bcon_output_enable_data = (long long)0x3ffffffffff;
		bist_flag = 1;
	}
	else if(!strncmp(ptr, "biststop", 8))
	{
		//bcon_mode_osg_data = 0;
		//bcon_mode_direction = 0;
		//bcon_led_on_off = 0;
		//bist_mode_use = (long long) 0x0000000000000000;
		//bcon_mode_output_enable_mask =(long long) 0x0000000000000000;
		//bcon_output_enable_data = (long long) 0x0000000000000000;
		bist_flag = 1;
	}
	else if(!strncmp(ptr, "bistvalue", 9))
	{
		printf("BIST RESULT VALUE\r\n");
		for(int i=0; i<42; i++)
		{
			printf("[CH%02d] = %d\r\n",i+1,bist_adc_value[i]);
		}
	}
	else if(!strncmp(ptr, "bistcalinit", 11))
	{
		memset(&bist_cal, 0, sizeof(bist_cal));
		printf("bist_0v_value = %f\r\n", bist_cal.bist_0v_value);
		printf("bist_0v_step = %f\r\n", bist_cal.bist_0v_step);
		printf("bist_15v_value = %f\r\n", bist_cal.bist_15v_value);
		printf("bist_15v_step = %f\r\n", bist_cal.bist_15v_step);
		printf("bist_ratio = %f\r\n", bist_cal.bist_ratio);
		printf("bist_offset = %f\r\n", bist_cal.bist_offset);
		printf("bist_user_offset = %f\r\n", bist_cal.bist_user_offset);
	}
	else if(!strncmp(ptr, "bistcal0v ", 10))
	{
		bist_output_enable(1);
		HAL_Delay(10);
		bist_cal.bist_0v_value = atof(ptr+10);
		// KGH FIX START
		while(HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK);
		// KGH FIX END
		if(HAL_ADC_Start(&hadc) == HAL_OK)
		{
			if(HAL_ADC_PollForConversion(&hadc,1000) == HAL_OK)
			{
				bist_cal.bist_0v_step = (double)HAL_ADC_GetValue(&hadc);
				printf("bist_0v_value = %f\r\n", bist_cal.bist_0v_value);
				printf("bist_0v_step = %f\r\n", bist_cal.bist_0v_step);
			}
			else printf("HAL_ADC_PollForConversion Fail\r\n");
		}
		else printf("HAL_Start Fail\r\n");

		while(HAL_ADC_Stop(&hadc) != HAL_OK);

		bist_output_enable((long long)0);
	}
	else if(!strncmp(ptr, "bistcal15v ", 11))
	{
		bist_output_enable(1);
		HAL_Delay(10);
		bist_cal.bist_15v_value = atof(ptr+11);
		// KGH FIX START
		while(HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK);
		// KGH FIX END
		if(HAL_ADC_Start(&hadc) == HAL_OK)
		{
			if(HAL_ADC_PollForConversion(&hadc,1000) == HAL_OK)
			{
				bist_cal.bist_15v_step = (double)HAL_ADC_GetValue(&hadc);
				printf("bist_15v_value = %f\r\n", bist_cal.bist_15v_value);
				printf("bist_15v_step = %f\r\n", bist_cal.bist_15v_step);
			}
			else printf("HAL_ADC_PollForConversion Fail\r\n");
		}
		else printf("HAL_Start Fail\r\n");

		while(HAL_ADC_Stop(&hadc) != HAL_OK);

		bist_output_enable((long long)0);
	}
	else if(!strcmp(ptr, "bistcal"))
	{
		bist_cal.bist_ratio = (bist_cal.bist_15v_value - bist_cal.bist_0v_value) / (bist_cal.bist_15v_step - bist_cal.bist_0v_step);
		bist_cal.bist_offset = bist_cal.bist_0v_value - (bist_cal.bist_ratio * bist_cal.bist_0v_step);
		printf("bist_ratio = %f\r\n", bist_cal.bist_ratio);
		printf("bist_offset = %f\r\n", bist_cal.bist_offset);
	}
	else if(!strncmp(ptr, "bistoffset ", 11))
	{
		bist_cal.bist_user_offset = atof(ptr+11);
		printf("bist_user_offset = %f\r\n", bist_cal.bist_user_offset);
	}
	else if(!strncmp(ptr, "bistprint", 9))
	{
		printf("bist_0v_value = %f\r\n", bist_cal.bist_0v_value);
		printf("bist_0v_step = %f\r\n", bist_cal.bist_0v_step);
		printf("bist_15v_value = %f\r\n", bist_cal.bist_15v_value);
		printf("bist_15v_step = %f\r\n", bist_cal.bist_15v_step);
		printf("bist_ratio = %f\r\n", bist_cal.bist_ratio);
		printf("bist_offset = %f\r\n", bist_cal.bist_offset);
		printf("bist_user_offset = %f\r\n", bist_cal.bist_user_offset);
	}
	else if(!strncmp(ptr, "bistcalsave", 11))
	{
		if(FLASH_If_Erase(OFFSET_ADDRESS, sizeof(bist_cal)) == HAL_OK)
		{
			if(FLASH_If_Write(OFFSET_ADDRESS, (uint32_t *)(&bist_cal), sizeof(bist_cal)) == HAL_OK)
			{
				printf("FLASH WRITE SUCCESS\r\n");
			}
			else printf("FLASH WRITE FAIL\r\n");
		}
		else printf("FLASH ERASE FAIL\r\n");
	}
	else if(!strncmp(ptr, "aaa", 3))
	{
		bist_output_enable((long long)0x0000000000000001);
		printf("TEST\r\n");
	}
	else if(!strncmp(ptr, "bbb", 3))
	{
		bist_output_enable((long long)0x0000000000000000);
		printf("TEST\r\n");
	}
	else if(!strncmp(ptr, "ccc", 3))
	{
		bist_cnt = 0;
		bist_flag = 1;
		printf("RECV BIST START\r\n");
	}

	else if(!strncmp(ptr, "adccal", 6))
	{
		//while(HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK);
		if(HAL_ADCEx_Calibration_Start(&hadc) == HAL_OK) printf("ADC CAL OK\r\n");
		else printf("ADC CAL NG\r\n");
	}
	else if(!strncmp(ptr, "out ", 4) && (strlen(ptr)==15))
	{
	    bcon_output_enable_data = strtoll(ptr+4,NULL,16) & ((long long)0x3ffffffffff);
		printf("BCON MODE OUTPUT USE CHANNEL ENABLE(1)/DISABLE(0) = 0x%llx\r\n",bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "out on", 6))
	{;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "out off", 7))
	{
		bcon_output_enable(0x0000000000000000);
	}
	else if(!strncmp(ptr, "bistuse ", 8) && (strlen(ptr)==19))
	{
		bist_mode_use = strtoll(ptr+8,NULL,16)& ((long long)0x3ffffffffff);
		printf("BIST MODE USE(1)/NOT USE(0) = 0x%llx\r\n",bist_mode_use);
	}
/*	else if(!strncmp(ptr, "bistadc ", 8) && (strlen(ptr)==19))
	{
		bist_mode_adc_dac = ((strtoll(ptr+8,NULL,16)) & ((long long)0x3ffffffffff));
		printf("BIST MODE ADC(0)/DAC(1) = 0x%llx\r\n",(long long)bist_mode_adc_dac);
	}*/
	else if(!strncmp(ptr, "485test ", 7))
	{
		unsigned char arr[10] = "485TEST_OK";
		HAL_UART_Transmit(&COM_UART_PORT,arr , 10, 100);
	}
	else if(!strncmp(ptr, "ggggggg", 7))
	{
		char str[128] = {0};
		char strt[1024] = {0};
		float test_value_2 = 0;
		float test_value = 0;
		//sprintf(str,"+2.85945753E-01");
		sprintf(str,"-6.32467775E-01");
		test_value = atof(str);
		printf("TP1 = %f\r\n", test_value);

		memcpy(&strt[0], &test_value,sizeof(test_value));
		memcpy(&test_value_2, &strt,sizeof(test_value_2));
		printf("TP2=%f\r\n", test_value_2);
	}
	else if(!strncmp(ptr, "aaa", 3))
	{
		//com_task_ack(KEITHLEY_READ_REQ);
		printf("TP1 = %d\r\n", sizeof(bist_config));
	}
	else if(!strcmp(ptr, "ad"))
	{
		DEBUG_UART_PORT.Init.BaudRate = 115200;
		if(UART_SetConfig(&DEBUG_UART_PORT) == HAL_OK) printf("Uart Set OK\r\n");
		else printf("Uart Set Fail\r\n");
	}
	else if(!strncmp(ptr, "ae", 2))
	{
		DEBUG_UART_PORT.Init.BaudRate = 19200;
		if(UART_SetConfig(&DEBUG_UART_PORT) == HAL_OK) printf("Uart Set OK\r\n");
		else printf("Uart Set Fail\r\n");
	}
	else if(!strncmp(ptr, "bbb", 3))
	{
		HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_RESET);
		if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("fail\r\n");
		else printf("OKOK\r\n");
		HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_SET);
		if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("fail\r\n");
		else printf("OKOK\r\n");
	}
	else if(!strncmp(ptr, "ccc", 3))
	{
		HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_SET);
		if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("fail\r\n");
		else printf("OKOK\r\n");
		HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_RESET);
		if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("fail\r\n");
		else printf("OKOK\r\n");
	}
	else if(!strncmp(ptr, "bcon-data",9 ))
	{
		bcon_mode_select(DATA_SSD, FORWARD, LED_ON, ALL_ON);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = DATA_SSD;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "bcon-osg",8 ))
	{
		bcon_mode_select(OSG_12CLK, FORWARD, LED_ON, ALL_ON);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = OSG_12CLK;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "bcon-rever",10))
	{
		bcon_mode_select(OSG_12CLK, REVERSE, LED_ON, ALL_ON);
		bcon_mode_direction = REVERSE;
		bcon_mode_osg_data = OSG_12CLK;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "bcon-none",9))
	{
		bcon_mode_select(OSG_12CLK, FORWARD, LED_ON, ALL_OFF);
		bcon_mode_direction = REVERSE;
		bcon_mode_osg_data = OSG_12CLK;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "osg6l",5))
	{
		bcon_mode_select(OSG_6CLK, FORWARD, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = OSG_6CLK;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "osg6r",4))
	{
		bcon_mode_select(OSG_6CLK, REVERSE, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = REVERSE;
		bcon_mode_osg_data = OSG_6CLK;
		bcon_output_enable(bcon_output_enable_data);
	}
	else if(!strncmp(ptr, "dataosg",7))
	{
		bcon_mode_select(DATA_OSG, FORWARD, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = DATA_OSG;
		bcon_output_enable(bcon_output_enable_data);
		printf("DATAOSG\r\n");
	}
	else if(!strncmp(ptr, "data77",6))
	{
		bcon_mode_select(DATA_77, FORWARD, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = DATA_77;
		bcon_output_enable(bcon_output_enable_data);
		printf("DATA77\r\n");
	}
	else if(!strncmp(ptr, "datanew",7))
	{
		bcon_mode_select(DATA_NEW, FORWARD, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = DATA_NEW;
		bcon_output_enable(bcon_output_enable_data);
		printf("DATANEW\r\n");
	}
	else if(!strncmp(ptr, "datassd",7))
	{
		bcon_mode_select(DATA_SSD, FORWARD, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = DATA_SSD;
		bcon_output_enable(bcon_output_enable_data);
		printf("DATASSD\r\n");
	}
	else if(!strncmp(ptr, "osg12r",6))
	{
		bcon_mode_select(OSG_12CLK, REVERSE, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = REVERSE;
		bcon_mode_osg_data = OSG_12CLK;
		bcon_output_enable(bcon_output_enable_data);
		printf("OSG12CLK_R\r\n");
	}
	else if(!strncmp(ptr, "osg12l",6))
	{
		bcon_mode_select(OSG_12CLK, FORWARD, LED_ON, bcon_mode_output_enable_mask);
		bcon_mode_direction = FORWARD;
		bcon_mode_osg_data = OSG_12CLK;
		bcon_output_enable(bcon_output_enable_data);
		printf("OSG12CLK_L\r\n");

	}

	else if(!strncmp(ptr, "adc", 3))
	{
		//bcon_mode_osg_data = OSG_12CLK;
		//bcon_mode_direction = FORWARD;
		//bcon_led_on_off = LED_ON;
		bist_mode_use = (long long)0x0000000000000004;
		//bcon_mode_output_enable_mask = (long long)0x0000000000000004;
		//bcon_output_enable_data = (long long)0x0000000000000004;
		bist_flag2 = 1;
	}
	else if(!strncmp(ptr, "offadc", 6))
	{
		bcon_mode_osg_data = OSG_12CLK;
		bcon_mode_direction = FORWARD;
		bcon_led_on_off = LED_ON;
		bist_mode_use = (long long)0x0000000000000000;
		bcon_mode_output_enable_mask = (long long)0x0000000000000000;
		bcon_output_enable_data = (long long)0x0000000000000000;
		bist_flag2 = 0;
	}
	else if(!strncmp(ptr, "idcheck", 7))
	{
		printf("TP1 = %d\r\n",HAL_GPIO_ReadPin(ID5_GPIO_Port, ID5_Pin));
		printf("TP2 = %d\r\n",HAL_GPIO_ReadPin(ID1_GPIO_Port, ID4_Pin));
		printf("TP3 = %d\r\n",HAL_GPIO_ReadPin(ID1_GPIO_Port, ID3_Pin));
	}
	else if(!strncmp(ptr, "uid", 3))
	{
		int UID[3];

		UID[0] = *(__IO int*)UID_BASE;
		UID[1] = *(__IO int*)(UID_BASE+4);
		UID[2] = *(__IO int*)(UID_BASE+8);
		printf("%x,%x,%x\r\n", UID[0],UID[1],UID[2]);

	}
	else if(!strncmp(ptr, "qqq", 3))
	{
		bist_output_enable((long long)0x0000000000000004);
		if(HAL_ADC_Start(&hadc) == HAL_OK)
		{
			if(HAL_ADC_PollForConversion(&hadc,1000) == HAL_OK)
			{
				adc_value1 = ((((double)HAL_ADC_GetValue(&hadc)) - 332) * 0.1018507704) + bist_offset/10;
				if(adc_value1 >= 255) adc_value1 = 255;
				else if(adc_value1 < 0) adc_value1 = 0;
				//bist_adc_value[j] = (unsigned char)adc_value;
				printf("TP1 = %d\r\n",  (unsigned char)adc_value1);
			}
			else
			{
				if(bprf) printf("HAL_ADC_PollForConversion Fail\r\n");

			}
		}
		else
		{
			if(bprf) printf("HAL_ADC_Start Fail\r\n");
		}
		while(HAL_ADC_Stop(&hadc) != HAL_OK);
	}
	else
	{
		printf("Command Not Found\r\n");
	}
}
#endif

#if defined(COM_UART_MODE_POLLING) || defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA)
static void com_task(char *ptr)
{
	//char str[40];
	int i=0;
	unsigned short checksum_check=0;
	unsigned short recv_checksum=0;
	unsigned char power_on_off=0;

	if(!strncmp(ptr, "rs485", 5))
	{
		printf("RS485_RX_OK\r\n");
	}
	else
	{
		memcpy(&packet, ptr, sizeof(packet));
		if(packet.length_check >= 1033)
		{
			//printf("TP1 = %d\r\n", packet.length_check);
			return;
		}
		if(com_put_cnt >= (sizeof(packet) + 2))
		{
			memcpy(&recv_checksum, ptr+sizeof(packet)+packet.length_check, sizeof(recv_checksum));
			for(i=0; i<(com_put_cnt-2); i++)
			{
				checksum_check += (*(ptr+i)) & 0xff;
			}
		}

		if(cprf)
		{
			Cprintf("RECV BASE ID =                                                                                                    0x%02x\r\n",RECV_BCON_BASE_ID);
			Cprintf("RECV BCON ID =                                                                                                    0x%02x\r\n",RECV_BCON_ID);
			Cprintf("RECV CMD =                                                                                                        0x%04x\r\n",packet.cmd_check);
			Cprintf("RECV LENGTH =                                                                                                     0x%04x\r\n",packet.length_check);
			Cprintf("RECV CHECKSUM =                                                                                                   0x%04x\r\n",recv_checksum);
			Cprintf("Calculation CHECKSUM =                                                                                            0x%04x\r\n",checksum_check);
		}
		if(recv_checksum == checksum_check)
		{

			if(((RECV_BCON_BASE_ID == BRODCAST_ID) && (RECV_BCON_ID == BRODCAST_ID)) || ((RECV_BCON_BASE_ID == BCON_BASE_ID) && (RECV_BCON_ID == BCON_ID)))
			{
				if((packet.cmd_check == MODEL_CONFIG) && (packet.length_check == MODEL_CONFIG_LEN))
				{
					memset(&model_config, 0, sizeof(model_config));
					memcpy(&model_config, ptr+sizeof(packet), MODEL_CONFIG_LEN);
					bcon_mode_osg_data = (uint8_t)model_config.bcon_config[SELECT_MACRO].type;
					bcon_mode_direction = (uint8_t)model_config.bcon_config[SELECT_MACRO].direction;
					bcon_led_on_off = (uint8_t)model_config.led_on_off;
					memcpy(&bcon_mode_output_enable_mask, model_config.bcon_config[SELECT_MACRO].bcon_output_enable_mask_1, 8);
					bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_led_on_off,bcon_mode_output_enable_mask);
					Cprintf("RECV BCON MODE NONE(0)/O_DATA(1)/S_DATA(2)/O_12K(3)/O_8K(4)/O_6K(5),FORWARD(0)/REVERSE(1), LED_OFF(0)/LED_ON(1) = %d\r\n",(unsigned char)bcon_mode_osg_data);
					Cprintf("RECV BCON MODE FORWARD(0)/REVERSE(1) =                                                                            %d\r\n",(unsigned char)bcon_mode_direction);
					Cprintf("RECV BCON MODE LED_OFF(0)/LED_ON(1) =                                                                             %d\r\n",(unsigned char)bcon_led_on_off);
					Cprintf("RECV BCON MODE OUTPUT MASK OFF(0)/ON(1)=                                                                          %llx\r\n",(long long)bcon_mode_output_enable_mask);
				}
				else if((packet.cmd_check == BIST_CONFIG) && (packet.length_check == BIST_CONFIG_LEN))
				{
					memset(&bist_config, 0, sizeof(bist_config));
					memcpy(&bist_config, ptr+sizeof(packet), BIST_CONFIG_LEN);
					memcpy(&bist_mode_use, bist_config[SELECT_MACRO].bist_adc_type,8);
					Cprintf("RECV BIST MODE USE(1)/NOT USE(0) = 0x%llx\r\n",(long long)bist_mode_use);
				}
				else if((packet.cmd_check == BIST_START) && (packet.length_check == BIST_START_LEN))
				{
					bist_cnt = 0;
					bist_flag = 1;
					Cprintf("RECV BIST START\r\n");
				}
				else if((packet.cmd_check == BIST_STATUS_REQ) && (packet.length_check == BIST_STATUS_REQ_LEN))
				{
					com_task_ack(packet.cmd_check);
					Cprintf("RECV BIST STATUS REQUEST\r\n");
				}
				else if((packet.cmd_check == BCON_OUTPUT_ON_OFF) && (packet.length_check == BCON_OUTPUT_ON_OFF_LEN))
				{
					power_on_off = (unsigned char)(*(ptr+sizeof(packet)));
					if(power_on_off == 1)
					{
						bcon_output_enable(bcon_output_enable_data);
						Cprintf("RECV ALL BCON OUTPUT ON\r\n");
					}
					else if(power_on_off == 0)
					{
						bcon_output_enable(0);
						Cprintf("RECV ALL BCON OUTPUT OFF\r\n");
					}
					else
					{
						Cprintf("RECV ALL BCON OUTPUT INVALID\r\n");
					}
				}
				else if((packet.cmd_check == BCON_SELECT_OUTPUT_ON_OFF) && (packet.length_check == BCON_SELECT_OUTPUT_ON_OFF_LEN))
				{
					memset(bcon_output_on_off_select, 0, sizeof(bcon_output_on_off_select));
					memcpy(bcon_output_on_off_select, ptr+sizeof(packet), BCON_SELECT_OUTPUT_ON_OFF_LEN);
					if(bcon_output_on_off_select[SELECT_MACRO] == 1)
					{
						bcon_output_enable(bcon_output_enable_data);
						Cprintf("RECV SELECT BCON OUTPUT ON\r\n");
					}
					else if(bcon_output_on_off_select[SELECT_MACRO] == 0)
					{
						bcon_output_enable(0);
						Cprintf("RECV SELECT BCON OUTPUT OFF\r\n");
					}
					else
					{
						Cprintf("RECV SELECT BCON OUTPUT INVALID\r\n");
					}
				}
				else if((packet.cmd_check == FW_VERSION_REQ) && (packet.length_check == FW_VERSION_REQ_LEN))
				{
					com_task_ack(packet.cmd_check);
					Cprintf("RECV FW VERSION REQUEST\r\n");
				}
				else if((packet.cmd_check == BCON_SELECT_OUTPUT_ON_OFF) && (packet.length_check == BCON_SELECT_OUTPUT_ON_OFF_LEN))
				{
					memset(bcon_output_on_off_select, 0, sizeof(bcon_output_on_off_select));
					memcpy(bcon_output_on_off_select, ptr+sizeof(packet), BCON_SELECT_OUTPUT_ON_OFF_LEN);
					if(bcon_output_on_off_select[SELECT_MACRO] == 1)
					{
						bcon_output_enable(bcon_output_enable_data);
						Cprintf("RECV SELECT BCON OUTPUT ON\r\n");
					}
					else if(bcon_output_on_off_select[SELECT_MACRO] == 0)
					{
						bcon_output_enable(0);
						Cprintf("RECV SELECT BCON OUTPUT OFF\r\n");
					}
					else
					{
						Cprintf("RECV SELECT BCON OUTPUT INVALID\r\n");
					}
				}
				else if((packet.cmd_check == KEITHLEY_READ_REQ) && (packet.length_check == KEITHLEY_READ_REQ_LEN)){

					uint8_t temp_str[64] = {0};
					int i = 0;
					DEBUG_UART_PORT.Init.BaudRate = 19200;
					UART_SetConfig(&DEBUG_UART_PORT);
					printf(":READ?\r");
					/*sprintf(str,":READ?%c",0x0d);
					while(HAL_UART_Transmit(&DEBUG_UART_PORT, (uint8_t *)str, 7, 100) != HAL_OK);*/

					while(1)
					{
						if(HAL_UART_Receive(&DEBUG_UART_PORT, (temp_str+(i++)), 1, 100) != HAL_OK) break;
					}
					keithley_data_conv_kgh((char *)temp_str);
					DEBUG_UART_PORT.Init.BaudRate = 115200;
					UART_SetConfig(&DEBUG_UART_PORT);
				}
				else if((packet.cmd_check == KEITHLEY_READ_REQ_ACK) && (packet.length_check == KEITHLEY_READ_REQ_ACK_LEN))
				{
					keithley_data = 0;
					memcpy(&keithley_data, ptr+sizeof(packet), KEITHLEY_READ_REQ_ACK_LEN);

					Cprintf("KEITHLEY = %f\r\n", keithley_data);

				}
				else if((packet.cmd_check == KEITHLEY_READ_START) && (packet.length_check == KEITHLEY_READ_START_LEN))
				{
					ssd_data_cal_flag = 1;
				}
				else if((packet.cmd_check == KEITHLEY_READ_END) && (packet.length_check == KEITHLEY_READ_END_LEN))
				{
					ssd_data_cal_flag = 0;
				}
				else if((packet.cmd_check == BCON_FWDOWNLOAD)&& (packet.length_check == BCON_FWDOWNLOAD_LEN))
				{
					memset(&packet_fw, 0, sizeof(packet_fw));
					memcpy(&packet_fw, ptr+sizeof(packet), packet.length_check);
					BCON_FW_DOWNLOAD();
				}
				else if((packet.cmd_check == BIST_OFFSET) && (packet.length_check == BIST_OFFSET_LEN))
				{
					memcpy(&bist_cal.bist_user_offset, ptr+sizeof(packet), packet.length_check);
					Cprintf("bist_user_offset = %f\r\n", bist_cal.bist_user_offset);

					if(FLASH_If_Erase(OFFSET_ADDRESS, sizeof(bist_cal)) == HAL_OK)
					{
						if(FLASH_If_Write(OFFSET_ADDRESS, (uint32_t *)(&bist_cal), sizeof(bist_cal)) == HAL_OK)
						{
							Cprintf("FLASH WRITE SUCCESS\r\n");
						}
						else Cprintf("FLASH WRITE FAIL\r\n");
					}
					else Cprintf("FLASH ERASE FAIL\r\n");
				}
				else if((packet.cmd_check == OSG_L_MODE) && (packet.length_check == MODE_LEN))
				{
					bcon_mode_select(OSG_12CLK, FORWARD, LED_ON, bcon_mode_output_enable_mask);
					bcon_mode_direction = FORWARD;
					bcon_mode_osg_data = OSG_12CLK;
					bcon_output_enable(bcon_output_enable_data);
				}
				else if((packet.cmd_check == OSG_R_MODE) && (packet.length_check == MODE_LEN))
				{
					bcon_mode_select(OSG_12CLK, REVERSE, LED_ON, bcon_mode_output_enable_mask);
					bcon_mode_direction = REVERSE;
					bcon_mode_osg_data = OSG_12CLK;
					bcon_output_enable(bcon_output_enable_data);
				}
				else if((packet.cmd_check == DATA_NEW_MODE) && (packet.length_check == MODE_LEN))
				{
					bcon_mode_select(DATA_NEW, FORWARD, LED_ON, bcon_mode_output_enable_mask);
					bcon_mode_direction = FORWARD;
					bcon_mode_osg_data = DATA_NEW;
					bcon_output_enable(bcon_output_enable_data);
				}
				else if((packet.cmd_check == DATA_SSD_MODE) && (packet.length_check == MODE_LEN))
				{
					bcon_mode_select(DATA_SSD, FORWARD, LED_ON, bcon_mode_output_enable_mask);
					bcon_mode_direction = FORWARD;
					bcon_mode_osg_data = DATA_NEW;
					bcon_output_enable(bcon_output_enable_data);
				}
				else if((packet.cmd_check == DATA_77_MODE) && (packet.length_check == MODE_LEN))
				{
					bcon_mode_select(DATA_77, FORWARD, LED_ON, bcon_mode_output_enable_mask);
					bcon_mode_direction = FORWARD;
					bcon_mode_osg_data = DATA_77;
					bcon_output_enable(bcon_output_enable_data);
				}
				else if((packet.cmd_check == DATA_OSG_MODE) && (packet.length_check == MODE_LEN))
				{
					bcon_mode_select(DATA_OSG, FORWARD, LED_ON, bcon_mode_output_enable_mask);
					bcon_mode_direction = FORWARD;
					bcon_mode_osg_data = DATA_OSG;
					bcon_output_enable(bcon_output_enable_data);
				}
				else
				{
					Cprintf("RECV CMD or LENGTH ERROR\r\n");
				}
			}
			else
			{
				Cprintf("MY BCON BASE ID = %x\r\n",((bcon_id >> 8) & 0x00ff));
				Cprintf("MY BCON ID = %x\r\n",(bcon_id & 0x00ff));
				Cprintf("ID MISMATCH\r\n");

			}
		}
		else
		{
			Cprintf("CHECKSUM ERROR\r\n");
		}
	}
}

static void com_task_ack(uint16_t cmd)
{
	int i = 0;
	uint8_t str[COM_BUFFER_SIZE]={0};
	unsigned short checksum=0;
	int tx_cnt = 0;
	unsigned short command = 0;
	unsigned short len = 0;

	//STX
	str[0] = 0x37;
	str[1] = 0xa4;
	str[2] = 0xc2;
	str[3] = 0x95;
	//BCON ID
	str[4] = BCON_BASE_ID;
	str[5] = BCON_ID;
	tx_cnt = 6;
	if(cmd == BIST_STATUS_REQ)
	{
		//COMMAND
		command = BIST_STATUS_REQ_ACK;
		memcpy(&str[tx_cnt], &command, sizeof(command));
		tx_cnt += sizeof(command);
		//LENGTH
		len = BIST_STATUS_REQ_ACK_LEN;
		memcpy(&str[tx_cnt], &len, sizeof(len));
		tx_cnt += sizeof(len);
		//DATA
		memcpy(&str[tx_cnt], (uint8_t *)bist_adc_value, BIST_STATUS_REQ_ACK_LEN);
		tx_cnt += BIST_STATUS_REQ_ACK_LEN;
		//CHECKSUM
		for(i=4; i<tx_cnt; i++)
		{
			checksum += str[i] & 0xff;
		}
		memcpy(&str[tx_cnt], &checksum, sizeof(checksum));
		tx_cnt += sizeof(checksum);
		str[tx_cnt++] = 0x59;
		str[tx_cnt++] = 0x2c;
		str[tx_cnt++] = 0x4a;
		str[tx_cnt++] = 0x0d;
		if(cprf)
		{
			Cprintf("SEND DATA :");
			for(i=0; i<tx_cnt; i++)
			{
				Cprintf(" %02x",str[i]);
			}
			Cprintf("\r\n");
		}
		while(HAL_UART_Transmit(&COM_UART_PORT, str, tx_cnt, 100) != HAL_OK);
	}
	else if(cmd == FW_VERSION_REQ)
	{
		//COMMAND
		command = FW_VERSION_REQ_ACK;
		memcpy(&str[tx_cnt], &command, sizeof(command));
		tx_cnt += sizeof(command);
		//LENGTH
		len = FW_VERSION_REQ_ACK_LEN;
		memcpy(&str[tx_cnt], &len, sizeof(len));
		tx_cnt += sizeof(len);
		//DATA
		str[tx_cnt++] = FW_VERSION;
		//CHECKSUM
		for(i=4; i<tx_cnt; i++)
		{
			checksum += str[i] & 0xff;
		}
		memcpy(&str[tx_cnt], &checksum, sizeof(checksum));
		tx_cnt += sizeof(checksum);
		str[tx_cnt++] = 0x59;
		str[tx_cnt++] = 0x2c;
		str[tx_cnt++] = 0x4a;
		str[tx_cnt++] = 0x0d;
		if(cprf)
		{
			Cprintf("SEND DATA :");
			for(i=0; i<tx_cnt; i++)
			{
				Cprintf(" %02x",str[i]);
			}
			Cprintf("\r\n");
		}
		while(HAL_UART_Transmit(&COM_UART_PORT, str, tx_cnt, 100) != HAL_OK);
	}
	else if(cmd == KEITHLEY_READ_REQ_ACK)
	{
		//COMMAND
		command = KEITHLEY_READ_REQ_ACK;
		memcpy(&str[tx_cnt], &command, sizeof(command));
		tx_cnt += sizeof(command);
		//LENGTH
		len = KEITHLEY_READ_REQ_ACK_LEN;
		memcpy(&str[tx_cnt], &len, sizeof(len));
		tx_cnt += sizeof(len);
		//DATA
		memcpy(&str[tx_cnt], &keithley_read_value,sizeof(keithley_read_value));
		tx_cnt += sizeof(keithley_read_value);

		//CHECKSUM
		for(i=4; i<tx_cnt; i++)
		{
			checksum += str[i] & 0xff;
		}
		memcpy(&str[tx_cnt], &checksum, sizeof(checksum));
		tx_cnt += sizeof(checksum);
		str[tx_cnt++] = 0x59;
		str[tx_cnt++] = 0x2c;
		str[tx_cnt++] = 0x4a;
		str[tx_cnt++] = 0x0d;
		if(cprf)
		{
			Cprintf("SEND DATA :");
			for(i=0; i<tx_cnt; i++)
			{
				Cprintf(" %02x",str[i]);
			}
			Cprintf("\r\n");
		}
		while(HAL_UART_Transmit(&COM_UART_PORT, str, tx_cnt, 100) != HAL_OK);
	}
	else
	{
		Cprintf("ACK COMMAND ERROR\r\n");
	}
}


void printf_bin(long long data)
{
	int i = 0;
	printf("hexadecimal  = 0x%llx\r\n", data);
	printf("binary = ");
	for(i = 63 ; i >= 0 ; --i)
	{
		printf("%d", (char)((data >> i) & 1));
		if((i%8) == 0) printf("/");
	}
	printf("\r\n");
}

void BCON_FW_DOWNLOAD(void)
{
	HAL_IWDG_Refresh(&hiwdg);
	if(packet_fw.packet_number == 0)
	{
		if(FLASH_If_Erase(FW_UPDATA_ADDRESS, (uint32_t)packet_fw.packet_total_number*FW_PACKET_SIZE) == HAL_OK)
			{
				if(FLASH_If_Write(FW_UPDATA_ADDRESS, (uint32_t *)(&packet_fw.fw_data), FW_PACKET_SIZE/4) == HAL_OK)
				{
					Cprintf("Number %d/%d FLASH WRITE SUCCESS\r\n", packet_fw.packet_number+1,packet_fw.packet_total_number);
					fw_down_count += 1;
					fw_dw_operation = 1;
				}
				else Cprintf("Number %d/%d FLASH WRITE FAIL\r\n", packet_fw.packet_number+1,packet_fw.packet_total_number);
			}
			else Cprintf("FLASH ERASE FAIL\r\n");
	}
	else
	{
		if((packet_fw.packet_total_number-1) == packet_fw.packet_number)
		{
			printf("PACKET number == total_number\r\n");
			if(FLASH_If_Write(FW_UPDATA_ADDRESS+(packet_fw.packet_number*FW_PACKET_SIZE), (uint32_t *)(&packet_fw.fw_data), FW_PACKET_SIZE/4) == HAL_OK)
			{
				Cprintf("Number %d/%d FLASH WRITE SUCCESS\r\n", packet_fw.packet_number+1,packet_fw.packet_total_number);
				Cprintf("FWDOWNLOAD SUCCESS\r\n");
				if(fw_down_count == (packet_fw.packet_total_number-1))
				{
					fw_flag.fw_flag = 1;
					//fw_flag.number = packet_fw.packet_number;
					fw_flag.number = packet_fw.packet_total_number;
					if(FLASH_If_Erase(FW_UPDATA_FLAG, (uint32_t)sizeof(fw_flag)) == HAL_OK)
					{
						if(FLASH_If_Write(FW_UPDATA_FLAG, (uint32_t *)&fw_flag, (sizeof(fw_flag))/4) == HAL_OK)
						{
							Cprintf("FW FLAG FLASH WRITE SUCCESS\r\n");
							fw_down_count = 0;
							NVIC_SystemReset();

						}
						else Cprintf("FW FLAG FLASH WRITE FAIL\r\n");
					}
				}
				else Cprintf("packet transmission error\r\n");

			}
			else Cprintf("Number %d FLASH WRITE FAIL\r\n", packet_fw.packet_number);
		}
		else if((packet_fw.packet_total_number-1) > packet_fw.packet_number)
		{
			if(FLASH_If_Write(FW_UPDATA_ADDRESS+(packet_fw.packet_number*FW_PACKET_SIZE), (uint32_t *)(&packet_fw.fw_data), FW_PACKET_SIZE/4) == HAL_OK)
			{
				fw_down_count += 1;
				Cprintf("Number %d/%d FLASH WRITE SUCCESS\r\n", packet_fw.packet_number+1,packet_fw.packet_total_number);
			}
			else Cprintf("Number %d/%d FLASH WRITE FAIL\r\n", packet_fw.packet_number+1,packet_fw.packet_total_number);
		}
	}
}

void keithley_data_conv_kgh(char *str)
{

	keithley_read_value = atof(str);
	com_task_ack(KEITHLEY_READ_REQ_ACK);
}
#endif

