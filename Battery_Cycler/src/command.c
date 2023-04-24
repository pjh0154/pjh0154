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
uint8_t cprf=0;
#define Cprintf if(cprf) printf
NAND_HandleTypeDef hnand1;
int com_buffer_intterrupt_cnt;
int com_buffer_cnt;
uint8_t com_buffer_intterrupt[COM_BUFFER_SIZE];

//static void debug(char *ptr);
#endif
#if defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA) || defined(COM_UART_MODE_POLLING)
#define STX 0x02
#define ETX 0x03

//Protocol Definition
#define R_VSC_STS            0x120
#define R_ISC_STS            0x121
#define R_VLINK_STS          0x122
#define R_ILINK_STS          0x123
#define R_TEMP_BUCK_BOOST    0x124
#define R_SC_SOC_STS         0x125
#define R_BUCK_BOOST_STS     0x126
#define R_CYCLE_STEP_STS     0x127
#define R_MODE_TYPE_STS      0x128
#define R_ACDC_V_RST	 	 0x20
#define R_ACDC_I_RST		 0x21
#define R_ACDC_DC_ACF_TEMP   0x22
#define R_ACDC_POWER_0		 0x23
#define R_ACDC_POWER_1		 0x24
#define R_ACDC_FLAG_FAULT    0x25
#define R_FB_V_I			 0x40
#define R_FB_FAULT			 0x41
#define R_FB_STATUS			 0x45
#define RW_BUCK_BOOST_CMD    0x100
#define RW_SET_REPEAT		 0x101
#define RW_SET_TEST_SOC		 0x102
#define RW_SETCHANNEL_STEP	 0x103
#define RW_SET_SAFETY_0		 0x104
#define RW_SET_SAFETY_1 	 0x105
#define RW_SET_STEP_0		 0x106
#define RW_SET_STEP_1		 0x107
#define RW_SET_STEP_2		 0x108
#define RW_TEST				 0x18
#define RW_ACDC_PARAM_0		 0x10
#define RW_ACDC_PARAM_1		 0x11
#define RW_ACDC_PARAM_2		 0x12
#define RW_ACDC_PARAM_3		 0x13
#define RW_ACDC_PARAM_4		 0x14
#define RW_ACDC_PARAM_5		 0x15
#define RW_ACDC_PARAM_6		 0x16
#define RW_ACDC_COMMAND		 0x17
#define RW_Save_Par			 0x19
#define RW_FB_COMMAND_0		 0x42
#define RW_FB_COMMAND_1		 0x43
#define RW_FB_COMMAND_2		 0x44

static char com_buffer[COM_BUFFER_SIZE] = {0};
uint8_t com_put = 0;
static uint8_t com_put_cnt = 0;
static uint8_t com_packet_state = 0;
uint8_t psfb_flag = 0;
uint8_t acdc_flag = 0;
uint8_t dis_acdc_flag = 0;
uint8_t manual_flag = 0;
uint8_t acdc_ovp = 0;
uint8_t flag1 = 0;
uint8_t psfb_ovp = 0;
static void com_task(char *ptr);
HAL_StatusTypeDef com_tx(char *str);
char strstr1[256]={0};
int i=0;
#endif

extern UART_HandleTypeDef DEBUG_UART_PORT;
extern UART_HandleTypeDef COM_UART_PORT;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	#if defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA) || defined(DEBUG_UART_MODE_POLLING)
	if(huart->Instance == DEBUG_UART_PORT.Instance)
	{
		if((debug_put == '\r') || (debug_put == '\n'))
		{
			printf("\r\n");
			if(debug_put_cnt != 0) debug(debug_buffer);
			debug_put_cnt = 0;
			memset(debug_buffer, 0, sizeof(debug_buffer));
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
		if(com_put == STX)
		{
			com_packet_state = 1;
			com_put_cnt = 0;
			memset(com_buffer, 0, sizeof(com_buffer));
		}
		else if((com_put == ETX) && (com_packet_state == 1))
		{
			//int i=0;
			com_packet_state = 0;
			com_task(com_buffer);
/*			for(i=0; i<=com_put_cnt; i++)
			{
			printf("%c", com_buffer[i]);
			}
			i=0;
			printf("\r\n");*/
		}
		else
		{
			if(com_packet_state == 1) com_buffer[com_put_cnt++] = com_put;
			if(com_put_cnt >= COM_BUFFER_SIZE) com_packet_state = 0;
		}
		#ifdef COM_UART_MODE_INTERRUPT
		HAL_UART_Receive_IT(&COM_UART_PORT, &com_put, 1);
		#endif
	}
	#endif
}

#if defined(DEBUG_UART_MODE_POLLING) || defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA)
//static void debug(char *ptr)
void debug(char *ptr)
{
	if(!strncmp(ptr, "help", 4))
	{
		printf("============== Command List ==============\r\n");
		printf("Software Reset = reset\r\n");
		#if defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA) || defined(COM_UART_MODE_POLLING)
		printf("UART For Communication Port Test = testuart,(User_Text....)\r\n");
		#endif
		#if defined(CAN_MODE_INTERRUPT) || defined(CAN_MODE_POLLING)
		printf("CAN Master Tx Test = testcantx,(User_Text.... no more than 8bytes)\r\n");
		#endif
		printf("============== Command List ==============\r\n");
	}
	else if(!strncmp(ptr, "reset", 5))
	{
		NVIC_SystemReset();
	}
	#if defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA) || defined(COM_UART_MODE_POLLING)
	else if(!strncmp(ptr, "testuart,", 9))
	{
		char *str = strtok(ptr+9,NULL);
		if(com_tx(str) == HAL_OK) printf("COM TX Success\r\n");
		else printf("COM TX Fail\r\n");
	}
	#endif
	#if defined(CAN_MODE_INTERRUPT) || defined(CAN_MODE_POLLING)
	else if(!strncmp(ptr, "testcantx,", 10))
	{
		HAL_StatusTypeDef ret = HAL_ERROR;
		int i;
		char *temp=NULL;
		char temp_buffer[8] = {0};
		temp = strtok(ptr+10, NULL);
		for(i=0; i<strlen(temp); i++) temp_buffer[i] = *(temp+i);
		ret = can_tx(CAN_ID_STD, CAN_RTR_DATA, 0x10, 0x000, temp_buffer, strlen(temp));
		if(ret == HAL_OK)
		{
			printf("CAN TX SUCCESS\r\n");
		}
		else if(ret == HAL_BUSY)
		{
			printf("CAN TxMailBox NOT EMPTY\r\n");
		}
		else printf("CAN TX FAIL\r\n");
	}
	else if(!strncmp(ptr, "testcan,",7))
	{
		HAL_StatusTypeDef ret = HAL_ERROR;
		char cantest[8]={0x01,0x02,0x03,0x04};
		ret = can_tx(CAN_ID_STD, CAN_RTR_DATA, 0x10, 0x000, cantest, 8);
		if(ret == HAL_OK)
		{
			printf("CAN TX SUCCESS\r\n");
		}
		else if(ret == HAL_BUSY)
		{
			printf("CAN TxMailBox NOT EMPTY\r\n");
		}
		else printf("CAN TX FAIL\r\n");

	}
	else if(!strncmp(ptr, "cprf", 4))
	{
		if(cprf == 0)
		{
			cprf = 1;
			printf("Print ON\r\n");
		}
		else
		{
			cprf = 0;
			printf("Print OFF\r\n");
		}
	}
	#endif

	else if(!strncmp(ptr, "2", 1))
	{

		buck_boost[0].step_channel = 0;
		buck_boost[0].step_step_id = 1;
		buck_boost[0].norminal_voltage = 1000;
		buck_boost[0].norminal_capacity = 10;
		buck_boost[0].max_voltage = 2000;
		buck_boost[0].min_voltage = 500;
		buck_boost[0].max_current = 2000;
		buck_boost[0].min_current = 1000;
		buck_boost[0].max_soc = 10;
		buck_boost[0].min_soc = 5;
/*		buck_boost[0].step_mode_id = 0;
		buck_boost[0].step_type_id = 0;
		buck_boost[0].step_voltage = 1000;
		buck_boost[0].step_current = 1500;
		buck_boost[0].step_time = 100;
		buck_boost[0].step_extra = 100;
		buck_boost[0].step_termination_voltage = 1000;
		buck_boost[0].step_termination_current = 1000;
		buck_boost[0].step_termination_soc = 10;
		buck_boost[0].step_termination_time = 100;*/

		if(can_flag !=1)
		{
		while(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x103, 0x000, (char *)&buck_boost[0].step_channel, 2) != HAL_OK);
		while(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x104, 0x000, (char *)&buck_boost[0].norminal_voltage, 8) != HAL_OK);
		while(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x105, 0x000, (char *)&buck_boost[0].max_current, 8) != HAL_OK);
		}
	}
	else if(!strncmp(ptr, "buck1",5))
	{
						printf("\r\nSC_Voltage_1[1]=          %d\r\n",buck_boost[0].SC_Voltage_1);
						printf("SC_Voltage_2[1]=          %d\r\n",buck_boost[0].SC_Voltage_2);
						printf("SC_Voltage_3[1]=          %d\r\n",buck_boost[0].SC_Voltage_3);
						printf("SC_Voltage_4[1]=          %d\r\n",buck_boost[0].SC_Voltage_4);
						printf("SC_Current_1[1]=          %d\r\n",buck_boost[0].SC_Current_1);
						printf("SC_Current_2[1]=          %d\r\n",buck_boost[0].SC_Current_2);
						printf("SC_Current_3[1]=          %d\r\n",buck_boost[0].SC_Current_3);
						printf("SC_Current_4[1]=          %d\r\n",buck_boost[0].SC_Current_4);
						printf("Link_Voltage_1[1]=        %d\r\n",buck_boost[0].Link_Voltage_1);
						printf("Link_Voltage_2[1]=        %d\r\n",buck_boost[0].Link_Voltage_2);
						printf("Link_Voltage_3[1]=        %d\r\n",buck_boost[0].Link_Voltage_3);
						printf("Link_Voltage_4[1]=        %d\r\n",buck_boost[0].Link_Voltage_4);
						printf("Link_Current_1[1]=        %d\r\n",buck_boost[0].Link_Current_1);
						printf("Link_Current_2[1]=        %d\r\n",buck_boost[0].Link_Current_2);
						printf("Link_Current_3[1]=        %d\r\n",buck_boost[0].Link_Current_3);
						printf("Link_Current_4[1]=        %d\r\n",buck_boost[0].Link_Current_4);
						printf("Temp_1[1]=                %d\r\n",buck_boost[0].Temp_1);
						printf("Temp_2[1]=                %d\r\n",buck_boost[0].Temp_2);
						printf("Temp_3[1]=                %d\r\n",buck_boost[0].Temp_3);
						printf("Temp_4[1]=                %d\r\n",buck_boost[0].Temp_4);
						printf("SOC_1[1]=                 %d\r\n",buck_boost[0].SOC_1);
						printf("SOC_2[1]=                 %d\r\n",buck_boost[0].SOC_2);
						printf("SOC_3[1]=                 %d\r\n",buck_boost[0].SOC_3);
						printf("SOC_4[1]=                 %d\r\n",buck_boost[0].SOC_4);
						printf("Operation_State_1[1]=     %d\r\n",buck_boost[0].Operation_State_1);
						printf("Operation_State_2[1]=     %d\r\n",buck_boost[0].Operation_State_2);
						printf("Operation_State_3[1]=     %d\r\n",buck_boost[0].Operation_State_3);
						printf("Operation_State_4[1]=     %d\r\n",buck_boost[0].Operation_State_4);
						printf("Fault_State_1[1]=         %d\r\n",buck_boost[0].Fault_State_1);
						printf("Fault_State_2[1]=         %d\r\n",buck_boost[0].Fault_State_2);
						printf("Fault_State_3[1]=         %d\r\n",buck_boost[0].Fault_State_3);
						printf("Fault_State_4[1]=         %d\r\n",buck_boost[0].Fault_State_4);
						printf("Cycle_State_1[1]=         %d\r\n",buck_boost[0].Cycle_State_1);
						printf("Cycle_State_2[1]=         %d\r\n",buck_boost[0].Cycle_State_2);
						printf("Cycle_State_3[1]=         %d\r\n",buck_boost[0].Cycle_State_3);
						printf("Cycle_State_4[1]=         %d\r\n",buck_boost[0].Cycle_State_4);
						printf("Step_State_1[1]=          %d\r\n",buck_boost[0].Step_State_1);
						printf("Step_State_2[1]=          %d\r\n",buck_boost[0].Step_State_2);
						printf("Step_State_3[1]=          %d\r\n",buck_boost[0].Step_State_3);
						printf("Step_State_4[1]=          %d\r\n",buck_boost[0].Step_State_4);
						printf("Mode_ID_1[1]=             %d\r\n",buck_boost[0].Mode_ID_1);
						printf("Mode_ID_2[1]=             %d\r\n",buck_boost[0].Mode_ID_2);
						printf("Mode_ID_3[1]=             %d\r\n",buck_boost[0].Mode_ID_3);
						printf("Mode_ID_4[1]=             %d\r\n",buck_boost[0].Mode_ID_4);
						printf("Type_ID_1[1]=             %d\r\n",buck_boost[0].Type_ID_1);
						printf("Type_ID_2[1]=             %d\r\n",buck_boost[0].Type_ID_2);
						printf("Type_ID_3[1]=             %d\r\n",buck_boost[0].Type_ID_3);
						printf("Type_ID_4[1]=             %d\r\n",buck_boost[0].Type_ID_4);
	}
	else if(!strncmp(ptr, "buck2",5))
	{
						printf("\r\nSC_Voltage_1[2]=          %d\r\n",buck_boost[1].SC_Voltage_1);
						printf("SC_Voltage_2[2]=          %d\r\n",buck_boost[1].SC_Voltage_2);
						printf("SC_Voltage_3[2]=          %d\r\n",buck_boost[1].SC_Voltage_3);
						printf("SC_Voltage_4[2]=          %d\r\n",buck_boost[1].SC_Voltage_4);
						printf("SC_Current_1[2]=          %d\r\n",buck_boost[1].SC_Current_1);
						printf("SC_Current_2[2]=          %d\r\n",buck_boost[1].SC_Current_2);
						printf("SC_Current_3[2]=          %d\r\n",buck_boost[1].SC_Current_3);
						printf("SC_Current_4[2]=          %d\r\n",buck_boost[1].SC_Current_4);
						printf("Link_Voltage_1[2]=        %d\r\n",buck_boost[1].Link_Voltage_1);
						printf("Link_Voltage_2[2]=        %d\r\n",buck_boost[1].Link_Voltage_2);
						printf("Link_Voltage_3[2]=        %d\r\n",buck_boost[1].Link_Voltage_3);
						printf("Link_Voltage_4[2]=        %d\r\n",buck_boost[1].Link_Voltage_4);
						printf("Link_Current_1[2]=        %d\r\n",buck_boost[1].Link_Current_1);
						printf("Link_Current_2[2]=        %d\r\n",buck_boost[1].Link_Current_2);
						printf("Link_Current_3[2]=        %d\r\n",buck_boost[1].Link_Current_3);
						printf("Link_Current_4[2]=        %d\r\n",buck_boost[1].Link_Current_4);
						printf("Temp_1[2]=                %d\r\n",buck_boost[1].Temp_1);
						printf("Temp_2[2]=                %d\r\n",buck_boost[1].Temp_2);
						printf("Temp_3[2]=                %d\r\n",buck_boost[1].Temp_3);
						printf("Temp_4[2]=                %d\r\n",buck_boost[1].Temp_4);
						printf("SOC_1[2]=                 %d\r\n",buck_boost[1].SOC_1);
						printf("SOC_2[2]=                 %d\r\n",buck_boost[1].SOC_2);
						printf("SOC_3[2]=                 %d\r\n",buck_boost[1].SOC_3);
						printf("SOC_4[2]=                 %d\r\n",buck_boost[1].SOC_4);
						printf("Operation_State_1[2]=     %d\r\n",buck_boost[1].Operation_State_1);
						printf("Operation_State_2[2]=     %d\r\n",buck_boost[1].Operation_State_2);
						printf("Operation_State_3[2]=     %d\r\n",buck_boost[1].Operation_State_3);
						printf("Operation_State_4[2]=     %d\r\n",buck_boost[1].Operation_State_4);
						printf("Fault_State_1[2]=         %d\r\n",buck_boost[1].Fault_State_1);
						printf("Fault_State_2[2]=         %d\r\n",buck_boost[1].Fault_State_2);
						printf("Fault_State_3[2]=         %d\r\n",buck_boost[1].Fault_State_3);
						printf("Fault_State_4[2]=         %d\r\n",buck_boost[1].Fault_State_4);
						printf("Cycle_State_1[2]=         %d\r\n",buck_boost[1].Cycle_State_1);
						printf("Cycle_State_2[2]=         %d\r\n",buck_boost[1].Cycle_State_2);
						printf("Cycle_State_3[2]=         %d\r\n",buck_boost[1].Cycle_State_3);
						printf("Cycle_State_4[2]=         %d\r\n",buck_boost[1].Cycle_State_4);
						printf("Step_State_1[2]=          %d\r\n",buck_boost[1].Step_State_1);
						printf("Step_State_2[2]=          %d\r\n",buck_boost[1].Step_State_2);
						printf("Step_State_3[2]=          %d\r\n",buck_boost[1].Step_State_3);
						printf("Step_State_4[2]=          %d\r\n",buck_boost[1].Step_State_4);
						printf("Mode_ID_1[2]=             %d\r\n",buck_boost[1].Mode_ID_1);
						printf("Mode_ID_2[2]=             %d\r\n",buck_boost[1].Mode_ID_2);
						printf("Mode_ID_3[2]=             %d\r\n",buck_boost[1].Mode_ID_3);
						printf("Mode_ID_4[2]=             %d\r\n",buck_boost[1].Mode_ID_4);
						printf("Type_ID_1[2]=             %d\r\n",buck_boost[1].Type_ID_1);
						printf("Type_ID_2[2]=             %d\r\n",buck_boost[1].Type_ID_2);
						printf("Type_ID_3[2]=             %d\r\n",buck_boost[1].Type_ID_3);
						printf("Type_ID_4[2]=             %d\r\n",buck_boost[1].Type_ID_4);
	}
	else if(!strncmp(ptr, "buck3",5))
	{
						printf("\r\nSC_Voltage_1[3]=          %d\r\n",buck_boost[2].SC_Voltage_1);
						printf("SC_Voltage_2[3]=          %d\r\n",buck_boost[2].SC_Voltage_2);
						printf("SC_Voltage_3[3]=          %d\r\n",buck_boost[2].SC_Voltage_3);
						printf("SC_Voltage_4[3]=          %d\r\n",buck_boost[2].SC_Voltage_4);
						printf("SC_Current_1[3]=          %d\r\n",buck_boost[2].SC_Current_1);
						printf("SC_Current_2[3]=          %d\r\n",buck_boost[2].SC_Current_2);
						printf("SC_Current_3[3]=          %d\r\n",buck_boost[2].SC_Current_3);
						printf("SC_Current_4[3]=          %d\r\n",buck_boost[2].SC_Current_4);
						printf("Link_Voltage_1[3]=        %d\r\n",buck_boost[2].Link_Voltage_1);
						printf("Link_Voltage_2[3]=        %d\r\n",buck_boost[2].Link_Voltage_2);
						printf("Link_Voltage_3[3]=        %d\r\n",buck_boost[2].Link_Voltage_3);
						printf("Link_Voltage_4[3]=        %d\r\n",buck_boost[2].Link_Voltage_4);
						printf("Link_Current_1[3]=        %d\r\n",buck_boost[2].Link_Current_1);
						printf("Link_Current_2[3]=        %d\r\n",buck_boost[2].Link_Current_2);
						printf("Link_Current_3[3]=        %d\r\n",buck_boost[2].Link_Current_3);
						printf("Link_Current_4[3]=        %d\r\n",buck_boost[2].Link_Current_4);
						printf("Temp_1[3]=                %d\r\n",buck_boost[2].Temp_1);
						printf("Temp_2[3]=                %d\r\n",buck_boost[2].Temp_2);
						printf("Temp_3[3]=                %d\r\n",buck_boost[2].Temp_3);
						printf("Temp_4[3]=                %d\r\n",buck_boost[2].Temp_4);
						printf("SOC_1[3]=                 %d\r\n",buck_boost[2].SOC_1);
						printf("SOC_2[3]=                 %d\r\n",buck_boost[2].SOC_2);
						printf("SOC_3[3]=                 %d\r\n",buck_boost[2].SOC_3);
						printf("SOC_4[3]=                 %d\r\n",buck_boost[2].SOC_4);
						printf("Operation_State_1[3]=     %d\r\n",buck_boost[2].Operation_State_1);
						printf("Operation_State_2[3]=     %d\r\n",buck_boost[2].Operation_State_2);
						printf("Operation_State_3[3]=     %d\r\n",buck_boost[2].Operation_State_3);
						printf("Operation_State_4[3]=     %d\r\n",buck_boost[2].Operation_State_4);
						printf("Fault_State_1[3]=         %d\r\n",buck_boost[2].Fault_State_1);
						printf("Fault_State_2[3]=         %d\r\n",buck_boost[2].Fault_State_2);
						printf("Fault_State_3[3]=         %d\r\n",buck_boost[2].Fault_State_3);
						printf("Fault_State_4[3]=         %d\r\n",buck_boost[2].Fault_State_4);
						printf("Cycle_State_1[3]=         %d\r\n",buck_boost[2].Cycle_State_1);
						printf("Cycle_State_2[3]=         %d\r\n",buck_boost[2].Cycle_State_2);
						printf("Cycle_State_3[3]=         %d\r\n",buck_boost[2].Cycle_State_3);
						printf("Cycle_State_4[3]=         %d\r\n",buck_boost[2].Cycle_State_4);
						printf("Step_State_1[3]=          %d\r\n",buck_boost[2].Step_State_1);
						printf("Step_State_2[3]=          %d\r\n",buck_boost[2].Step_State_2);
						printf("Step_State_3[3]=          %d\r\n",buck_boost[2].Step_State_3);
						printf("Step_State_4[3]=          %d\r\n",buck_boost[2].Step_State_4);
						printf("Mode_ID_1[3]=             %d\r\n",buck_boost[2].Mode_ID_1);
						printf("Mode_ID_2[3]=             %d\r\n",buck_boost[2].Mode_ID_2);
						printf("Mode_ID_3[3]=             %d\r\n",buck_boost[2].Mode_ID_3);
						printf("Mode_ID_4[3]=             %d\r\n",buck_boost[2].Mode_ID_4);
						printf("Type_ID_1[3]=             %d\r\n",buck_boost[2].Type_ID_1);
						printf("Type_ID_2[3]=             %d\r\n",buck_boost[2].Type_ID_2);
						printf("Type_ID_3[3]=             %d\r\n",buck_boost[2].Type_ID_3);
						printf("Type_ID_4[3]=             %d\r\n",buck_boost[2].Type_ID_4);
	}
	else if(!strncmp(ptr, "buck4",5))
	{
						printf("\r\nSC_Voltage_1[4]=          %d\r\n",buck_boost[3].SC_Voltage_1);
						printf("SC_Voltage_2[4]=          %d\r\n",buck_boost[3].SC_Voltage_2);
						printf("SC_Voltage_3[4]=          %d\r\n",buck_boost[3].SC_Voltage_3);
						printf("SC_Voltage_4[4]=          %d\r\n",buck_boost[3].SC_Voltage_4);
						printf("SC_Current_1[4]=          %d\r\n",buck_boost[3].SC_Current_1);
						printf("SC_Current_2[4]=          %d\r\n",buck_boost[3].SC_Current_2);
						printf("SC_Current_3[4]=          %d\r\n",buck_boost[3].SC_Current_3);
						printf("SC_Current_4[4]=          %d\r\n",buck_boost[3].SC_Current_4);
						printf("Link_Voltage_1[4]=        %d\r\n",buck_boost[3].Link_Voltage_1);
						printf("Link_Voltage_2[4]=        %d\r\n",buck_boost[3].Link_Voltage_2);
						printf("Link_Voltage_3[4]=        %d\r\n",buck_boost[3].Link_Voltage_3);
						printf("Link_Voltage_4[4]=        %d\r\n",buck_boost[3].Link_Voltage_4);
						printf("Link_Current_1[4]=        %d\r\n",buck_boost[3].Link_Current_1);
						printf("Link_Current_2[4]=        %d\r\n",buck_boost[3].Link_Current_2);
						printf("Link_Current_3[4]=        %d\r\n",buck_boost[3].Link_Current_3);
						printf("Link_Current_4[4]=        %d\r\n",buck_boost[3].Link_Current_4);
						printf("Temp_1[4]=                %d\r\n",buck_boost[3].Temp_1);
						printf("Temp_2[4]=                %d\r\n",buck_boost[3].Temp_2);
						printf("Temp_3[4]=                %d\r\n",buck_boost[3].Temp_3);
						printf("Temp_4[4]=                %d\r\n",buck_boost[3].Temp_4);
						printf("SOC_1[4]=                 %d\r\n",buck_boost[3].SOC_1);
						printf("SOC_2[4]=                 %d\r\n",buck_boost[3].SOC_2);
						printf("SOC_3[4]=                 %d\r\n",buck_boost[3].SOC_3);
						printf("SOC_4[4]=                 %d\r\n",buck_boost[3].SOC_4);
						printf("Operation_State_1[4]=     %d\r\n",buck_boost[3].Operation_State_1);
						printf("Operation_State_2[4]=     %d\r\n",buck_boost[3].Operation_State_2);
						printf("Operation_State_3[4]=     %d\r\n",buck_boost[3].Operation_State_3);
						printf("Operation_State_4[4]=     %d\r\n",buck_boost[3].Operation_State_4);
						printf("Fault_State_1[4]=         %d\r\n",buck_boost[3].Fault_State_1);
						printf("Fault_State_2[4]=         %d\r\n",buck_boost[3].Fault_State_2);
						printf("Fault_State_3[4]=         %d\r\n",buck_boost[3].Fault_State_3);
						printf("Fault_State_4[4]=         %d\r\n",buck_boost[3].Fault_State_4);
						printf("Cycle_State_1[4]=         %d\r\n",buck_boost[3].Cycle_State_1);
						printf("Cycle_State_2[4]=         %d\r\n",buck_boost[3].Cycle_State_2);
						printf("Cycle_State_3[4]=         %d\r\n",buck_boost[3].Cycle_State_3);
						printf("Cycle_State_4[4]=         %d\r\n",buck_boost[3].Cycle_State_4);
						printf("Step_State_1[4]=          %d\r\n",buck_boost[3].Step_State_1);
						printf("Step_State_2[4]=          %d\r\n",buck_boost[3].Step_State_2);
						printf("Step_State_3[4]=          %d\r\n",buck_boost[3].Step_State_3);
						printf("Step_State_4[4]=          %d\r\n",buck_boost[3].Step_State_4);
						printf("Mode_ID_1[4]=             %d\r\n",buck_boost[3].Mode_ID_1);
						printf("Mode_ID_2[4]=             %d\r\n",buck_boost[3].Mode_ID_2);
						printf("Mode_ID_3[4]=             %d\r\n",buck_boost[3].Mode_ID_3);
						printf("Mode_ID_4[4]=             %d\r\n",buck_boost[3].Mode_ID_4);
						printf("Type_ID_1[4]=             %d\r\n",buck_boost[3].Type_ID_1);
						printf("Type_ID_2[4]=             %d\r\n",buck_boost[3].Type_ID_2);
						printf("Type_ID_3[4]=             %d\r\n",buck_boost[3].Type_ID_3);
						printf("Type_ID_4[4]=             %d\r\n",buck_boost[3].Type_ID_4);
	}
	else if(!strncmp(ptr, "tcpbuck",7))
	{
						printf("\r\nRun_Stop_1=                   %d\r\n",buck_boost[0].Run_Stop_1);
						printf("Run_Stop_2=                   %d\r\n",buck_boost[0].Run_Stop_2);
						printf("Run_Stop_3=                   %d\r\n",buck_boost[0].Run_Stop_3);
						printf("Run_Stop_4=                   %d\r\n",buck_boost[0].Run_Stop_4);
						printf("Cycle_SetRepeat=              %d\r\n",buck_boost[0].Cycle_SetRepeat);
						printf("step_SetRepeat=               %d\r\n",buck_boost[0].step_SetRepeat);
						printf("SOC_SetTest=                  %d\r\n",buck_boost[0].SOC_SetTest);
						printf("step_channel=                 %d\r\n",buck_boost[0].step_channel);
						printf("step_step_id=                 %d\r\n",buck_boost[0].step_step_id);
						printf("norminal_voltage=             %d\r\n",buck_boost[0].norminal_voltage);
						printf("norminal_capacity=            %d\r\n",buck_boost[0].norminal_capacity);
						printf("max_voltage=                  %d\r\n",buck_boost[0].max_voltage);
						printf("min_voltage=                  %d\r\n",buck_boost[0].min_voltage);
						printf("max_current=                  %d\r\n",buck_boost[0].max_current);
						printf("min_current=                  %d\r\n",buck_boost[0].min_current);
						printf("max_soc=                      %d\r\n",buck_boost[0].max_soc);
						printf("min_soc=                      %d\r\n",buck_boost[0].min_soc);
						printf("step_mode_id=                 %d\r\n",buck_boost[0].step_mode_id);
						printf("step_type_id=                 %d\r\n",buck_boost[0].step_type_id);
						printf("step_voltage=                 %d\r\n",buck_boost[0].step_voltage);
						printf("step_current=                 %d\r\n",buck_boost[0].step_current);
						printf("step_time=                    %d\r\n",buck_boost[0].step_time);
						printf("step_extra=                   %d\r\n",buck_boost[0].step_extra);
						printf("step_termination_voltage=     %d\r\n",buck_boost[0].step_termination_voltage);
						printf("step_termination_current=     %d\r\n",buck_boost[0].step_termination_current);
						printf("step_termination_soc=         %d\r\n",buck_boost[0].step_termination_soc);
						printf("step_termination_time=        %d\r\n",buck_boost[0].step_termination_time);


	}
	else if(!strncmp(ptr, "acdc",4))
	{
						printf("\r\nac_r_voltage=             %d\r\n",acdc.ac_r_voltage);
						printf("ac_s_voltage=             %d\r\n",acdc.ac_s_voltage);
						printf("ac_t_voltage=             %d\r\n",acdc.ac_t_voltage);
						printf("ac_r_current=             %d\r\n",acdc.ac_r_current);
						printf("ac_s_current=             %d\r\n",acdc.ac_s_current);
						printf("ac_t_current=             %d\r\n",acdc.ac_t_current);
						printf("ac_frequency_r=           %d\r\n",acdc.ac_frequency_r);
						printf("dc_voltage_r=             %d\r\n",acdc.dc_voltage_r);
						printf("dc_current=               %d\r\n",acdc.dc_current);
						printf("temperature=              %d\r\n",acdc.temperature);
						printf("\r\napparent_power=       %d\r\n",acdc.apparent_power);
						printf("active_power=             %d\r\n",acdc.active_power);
						printf("reactive_power=           %d\r\n",acdc.reactive_power);
						printf("power_factor=             %d\r\n",acdc.power_factor);
						printf("dc_power=                 %d\r\n",acdc.dc_power);
						printf("out_out_relay=            %d\r\n",acdc.out_out_relay);
						printf("out_init_relay=           %d\r\n",acdc.out_init_relay);
						printf("out_pwm12cs=              %d\r\n",acdc.out_pwm12cs);
						printf("out_pwm34sc=              %d\r\n",acdc.out_pwm34sc);
						printf("out_pwm56sc=              %d\r\n",acdc.out_pwm56sc);
						printf("out_fault_led=            %d\r\n",acdc.out_fault_led);
						printf("out_run_led=              %d\r\n",acdc.out_run_led);
						printf("out_ready_led=            %d\r\n",acdc.out_ready_led);
						printf("out_fan=                  %d\r\n",acdc.out_fan);
						printf("flag_ac_ready=            %d\r\n",acdc.flag_ac_ready);
						printf("flag_comm_ok=             %d\r\n",acdc.flag_comm_ok);
						printf("flag_dc_ready=            %d\r\n",acdc.flag_dc_ready);
						printf("flag_dclink_ok=           %d\r\n",acdc.flag_dclink_ok);
						printf("flag_pll_ok=              %d\r\n",acdc.flag_pll_ok);
						printf("flag_zerocross=           %d\r\n",acdc.flag_zerocross);
						printf("flag_run=                 %d\r\n",acdc.flag_run);
						printf("flag_stop=                %d\r\n",acdc.flag_stop);
						printf("flag_fault_stop=          %d\r\n",acdc.flag_fault_stop);
						printf("flag_pwm_on=              %d\r\n",acdc.flag_pwm_on);
						printf("flag_pwm_steady=          %d\r\n",acdc.flag_pwm_steady);
						printf("flag_dummy=               %d\r\n",acdc.flag_dummy);
						printf("flag_pretest=             %d\r\n",acdc.flag_pretest);
						printf("flag_testmode=            %d\r\n",acdc.flag_testmode);
						printf("flag_gc_mode=             %d\r\n",acdc.flag_gc_mode);
						printf("ovr=                      %d\r\n",acdc.ovr);
						printf("ovs=                      %d\r\n",acdc.ovs);
						printf("ovt=                      %d\r\n",acdc.ovt);
						printf("uvr=                      %d\r\n",acdc.uvr);
						printf("uvs=                      %d\r\n",acdc.uvs);
						printf("uvt=                      %d\r\n",acdc.uvt);
						printf("ocr=                      %d\r\n",acdc.ocr);
						printf("ocs=                      %d\r\n",acdc.ocs);
						printf("oct=                      %d\r\n",acdc.oct);
						printf("of=                       %d\r\n",acdc.of);
						printf("uf=                       %d\r\n",acdc.uf);
						printf("ocdc=                     %d\r\n",acdc.ocdc);
						printf("ovdc=                     %d\r\n",acdc.ovdc);
						printf("uvdc=                     %d\r\n",acdc.uvdc);
						printf("otsen=                    %d\r\n",acdc.otsen);
						printf("aciint=                   %d\r\n",acdc.aciint);
						printf("dcvint=                   %d\r\n",acdc.dcvint);
						printf("igbt1=                    %d\r\n",acdc.igbt1);
						printf("igbt2=                    %d\r\n",acdc.igbt2);
						printf("igbt3=                    %d\r\n",acdc.igbt3);
						printf("pwm_trip=                 %d\r\n",acdc.pwm_trip);
						printf("temp_sw1=                 %d\r\n",acdc.temp_sw1);
						printf("temp_sw2=                 %d\r\n",acdc.temp_sw2);
						printf("fan1_lock=                %d\r\n",acdc.fan1_lock);
						printf("fan2_lock=                %d\r\n",acdc.fan2_lock);
	}
	else if(!strncmp(ptr, "psfb",4))
	{
						printf("\r\fb_lv_v=            	 %d\r\n",psfb.fb_lv_v);
						printf("fb_lv_i=             	 %d\r\n",psfb.fb_lv_i);
						printf("fb_hv_v=             	 %d\r\n",psfb.fb_hv_v);
						printf("fb_hv_l=             	 %d\r\n",psfb.fb_hv_i);
						printf("fb_fault_1=              %d\r\n",psfb.fb_fault_1);
						printf("fb_fault_2=              %d\r\n",psfb.fb_fault_2);
						printf("fb_fault_3=              %d\r\n",psfb.fb_fault_3);
						printf("fb_fault_4=              %d\r\n",psfb.fb_fault_4);
	}
/*	else if(!strncmp(ptr, "a",1))
	{
		printf("flag1===================           %d\r\n", *(&acdc.flag1));
		printf("flag2===================          %d\r\n", *(&acdc.flag2));
		printf("flag3===================          %d\r\n", *(&acdc.flag3));
		printf("flag4===================          %d\r\n", *(&acdc.flag4));

	}*/
	else if(!strncmp(ptr, "nandwr",6))
	{
		flash_write();
	}
	else if(!strncmp(ptr, "nander,",7))
	{
		char *temp_ptr_sra = NULL;
		char *temp_ptr_sra_1 = NULL;
		int page[5] = {0,};

		strtok_r(ptr , ",",&temp_ptr_sra_1);

		temp_ptr_sra = strtok_r(NULL, ",",&temp_ptr_sra_1);

		page[0] = atoi(temp_ptr_sra);

		flash_Erase_d(page);
	}
	else if(!strncmp(ptr, "ab,",3))
	{
		char *temp_ptr_sra = NULL;
		char *temp_ptr_sra_1 = NULL;
		char *temp_ptr_sra_2 = NULL;
		char *temp_ptr_sra_3 = NULL;
		int page[5] = {0,};

		strtok_r(ptr , ",",&temp_ptr_sra_3);

		temp_ptr_sra = strtok_r(NULL, ",",&temp_ptr_sra_3);
		temp_ptr_sra_1 = strtok_r(NULL, ",",&temp_ptr_sra_3);
		temp_ptr_sra_2 = strtok_r(NULL, ",",&temp_ptr_sra_3);

		page[0] = atoi(temp_ptr_sra);
		page[1] = atoi(temp_ptr_sra_1);
		page[2] = atoi(temp_ptr_sra_2);

		printf("TP1 = %d / %d /  %d\r\n", page[0],page[1],page[2]);

		//flash_write_d(&page[0],&page[1],&page[2]);
	}

	else if(!strncmp(ptr, "nandrd",6))
	{
		flash_read();
	}
	else if(!strncmp(ptr, "nandid",6))
	{
		if(HAL_NAND_Reset(&hnand1) == HAL_OK) printf("NAND Reset\r\n");
		flash_id();
	}
	else if(!strncmp(ptr, "norid",5))
	{
		nor_flash_id();
	}
	else if(!strncmp(ptr, "norwr",5))
	{
		if(nor_flash_write()==HAL_OK)
		{
			printf("Norwr OK\r\n");
		}
	}
	else if(!strncmp(ptr, "norrd",5))
	{
		nor_flash_read();
	}
	else if(!strncmp(ptr, "norer",5))
	{
		nor_flash_erase();
	}
	else if(!strncmp(ptr, "ramwr",5))
	{
		sram_write();
	}
	else if(!strncmp(ptr, "ramw1",5))
	{
		sram_write1();
	}
	else if(!strncmp(ptr, "ramrd",5))
	{
		sram_read();
	}
	else if(!strncmp(ptr, "fwdn",4))
	{
		SerialDownload();
	}
	else if(!strncmp(ptr, "fwup",4))
	{
		SerialUpload();
	}
	else if(!strncmp(ptr, "tp1",3))
	{
		  HAL_GPIO_WritePin(FAN_ONOFF_GPIO_Port, FAN_ONOFF_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(GPIOI, MC_ONOFF_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(LV_LED_GPIO_Port, LV_LED_Pin, GPIO_PIN_SET);
		  HAL_GPIO_WritePin(HV_LED_GPIO_Port, HV_LED_Pin, GPIO_PIN_SET);

		  printf("TP1 = %d\r\n", HAL_GPIO_ReadPin(GPIOE, EMS_IN_Pin));
	}
	else if(!strncmp(ptr, "tp2",3))
	{
		  HAL_GPIO_WritePin(FAN_ONOFF_GPIO_Port, FAN_ONOFF_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOI, MC_ONOFF_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOI, LV_LED_Pin, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(HV_LED_GPIO_Port, HV_LED_Pin, GPIO_PIN_RESET);
	}
	else if(!strncmp(ptr, "tp3",3))
	{
		printf("TP1 = %d\r\n", HAL_GPIO_ReadPin(GPIOE, EMS_IN_Pin));
	}
	else if(!strncmp(ptr, "testest",8))
	{
		int i=0;
		char tmp[150];
		for(i=0 ; i<sizeof(tmp) ; i++)
		//for(i=0 ; i<100 ; i++)
		{
			printf("TP%d=%c\r\n", i,tmp[i]);
		}
	}
	else if(!strncmp(ptr, "actest",6))
	{
		acdc.dc_voltage_r = 37368;
		printf("TP=%d\r\n",acdc.dc_voltage_r);


	}
	else if(!strncmp(ptr, "acdc0ff",7))
	{
		acdc.run_stop = 0x0F;
		if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
	}
	else if(!strncmp(ptr, "rs",2))
	{
		char *strr = strstr1;
		while(i<202)
			{
				for(i=0; i<200; i++)
				{
					strstr1[i]='a';
				}
				if(i==200)
				{
					com_tx(strr);
					i=205;
					printf("TXOK\r\n");
				}
			}
		i=0;
	}
	else if(!strncmp(ptr, "pstest",6))
	{
		printf("fb_lv_v =    %x\r\n", psfb.fb_lv_v);
		printf("fb_lv_i =    %x\r\n", psfb.fb_lv_i);
		printf("fb_hv_v =    %x\r\n", psfb.fb_hv_v);
		printf("fb_hv_i =    %x\r\n", psfb.fb_hv_i);
		printf("fb_lv_v_d =    %d\r\n", psfb.fb_lv_v);
		printf("fb_lv_i_d =    %d\r\n", psfb.fb_lv_i);
		printf("fb_hv_v_d =    %d\r\n", psfb.fb_hv_v);
		printf("fb_hv_i_d =    %d\r\n", psfb.fb_hv_i);
	}

	else if(!strncmp(ptr, "adrs,",5))
	{

		char *temp_ptr = NULL;
		char *test = NULL;
		int i = 0;

		__IO unsigned short temp_value[100] = {0};

		temp_ptr = strtok_r(ptr , ",",&test);
		while(temp_ptr != NULL)
		{
			if(i >= 1)	temp_value[i-1] = atoi(temp_ptr);
			//if(i >= 1) temp_value[i-1] = strtol(temp_ptr,NULL,10);
			i++;
			if(i == 3) break;
			temp_ptr = strtok_r(NULL, ",",&test);
		}
		printf("TP1 = %d / %d\r\n", temp_value[0], i-1);
		printf("TP2 = %d / %d\r\n", temp_value[1], i-1);

	}
}
#endif

#if defined(COM_UART_MODE_POLLING) || defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA)
static void com_task(char *ptr)
{
	if(!strncmp(ptr, "TEST", 4))
	{
		printf("uart2rx OK\r\n");
	}
	else if(!strncmp(ptr, "RST", 3))
	{
		NVIC_SystemReset();
	}
	else printf("UART Communication Protocol Not Found\r\n");
}

HAL_StatusTypeDef com_tx(char *str)
{
	HAL_StatusTypeDef result = HAL_ERROR;
	int i = 0;
	char str_tx[COM_BUFFER_SIZE+2] = {0};
	int len = strlen(str);

	str_tx[0] = STX;
	memcpy(&str_tx[1], str, len);
	str_tx[len+1] = ETX;

	HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
	while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET)
	{
		if(i++ > 1000)
		{
			return HAL_ERROR;
		}
	}
	result = HAL_UART_Transmit(&COM_UART_PORT, (uint8_t *)str_tx, len+2, 100);
	HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
	i=0;
	while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET)
	{
		if(i++ > 1000)
		{
			return HAL_ERROR;
		}
	}
	return result;
}
#endif

void charge_acdc(void)
{
	unsigned short acdc_sensing;
	acdc_sensing = (acdc.dc_voltage_r-32768)/10;


	if(380 <acdc_sensing && acdc_sensing <450)
	{
		printf("acdc ok\r\n");
		psfb.fb_buck_charging = psfb_on;
		if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
		psfb_flag = 1;
		acdc_flag = 0;
		psfb_ovp = 1;
	}
	else
	{
		printf("acdc ng\r\n");
		acdc.run_stop = acdc_off;
		if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
		acdc_flag = 0;
		psfb_flag = 0;
		manual_flag = 0;
		flag1 = 0;
	}
}
void charge_psfb(void)
{
	if(80 < psfb.fb_lv_v && psfb.fb_lv_v <150)
	{
		printf("psfb ok\r\n");
		if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
		psfb_flag = 0;
		//manual_flag = 1;
	}
	else
	{
		printf("psfb ng\r\n");
		acdc.run_stop = acdc_off;
		if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
		psfb.fb_buck_charging = psfb_off;
		if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
		acdc_flag = 0;
		psfb_flag = 0;
		manual_flag = 0;
		flag1 = 0;
	}
}
void auto_off(void)
{
	acdc.run_stop = acdc_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
	psfb.fb_buck_charging = psfb_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
	buck_boost[0].Run_Stop_1 = 0;
	buck_boost[0].Run_Stop_2 = 0;
	buck_boost[0].Run_Stop_3 = 0;
	buck_boost[0].Run_Stop_4 = 0;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
	acdc_flag = 0;
	psfb_flag = 0;
	manual_flag = 0;
	flag1 = 0;
	//printf("ACDC+DCDC+BUCK_BOOST OFF\r\n");
}
void auto_off1(void)
{
	acdc.run_stop = acdc_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
	psfb.fb_buck_charging = psfb_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
	buck_boost[1].Run_Stop_1 = 0;
	buck_boost[1].Run_Stop_2 = 0;
	buck_boost[1].Run_Stop_3 = 0;
	buck_boost[1].Run_Stop_4 = 0;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x200, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
	acdc_flag = 0;
	psfb_flag = 0;
	manual_flag = 0;
	flag1 = 0;
	//printf("ACDC+DCDC+BUCK_BOOST OFF\r\n");
}
void auto_off2(void)
{
	acdc.run_stop = acdc_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
	psfb.fb_buck_charging = psfb_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
	buck_boost[2].Run_Stop_1 = 0;
	buck_boost[2].Run_Stop_2 = 0;
	buck_boost[2].Run_Stop_3 = 0;
	buck_boost[2].Run_Stop_4 = 0;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x300, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
	acdc_flag = 0;
	psfb_flag = 0;
	manual_flag = 0;
	flag1 = 0;
	//printf("ACDC+DCDC+BUCK_BOOST OFF\r\n");
}
void auto_off3(void)
{
	acdc.run_stop = acdc_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
	psfb.fb_buck_charging = psfb_off;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
	buck_boost[3].Run_Stop_1 = 0;
	buck_boost[3].Run_Stop_2 = 0;
	buck_boost[3].Run_Stop_3 = 0;
	buck_boost[3].Run_Stop_4 = 0;
	if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x400, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
	acdc_flag = 0;
	psfb_flag = 0;
	manual_flag = 0;
	flag1 = 0;
	//printf("ACDC+DCDC+BUCK_BOOST OFF\r\n");
}

