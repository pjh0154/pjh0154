 /* can.c
 *
 *  Created on: 2019. 11. 18.
 *      Author: ghkim
 */

#include "can.h"

#if defined(CAN_MODE_INTERRUPT) || defined(CAN_MODE_POLLING)
extern CAN_HandleTypeDef CAN_PORT;
CAN_RxHeaderTypeDef RxMessage;
uint8_t can_recv_data[8];
uint16_t tcp_tx_data[4];
#define Cprintf if(cprf) printf
uint8_t cprf;
uint8_t can_flag=0;

HAL_StatusTypeDef CAN_ConfigFilter(void)
{
	CAN_FilterTypeDef sFilterConfig;
	HAL_StatusTypeDef status = HAL_ERROR;

	memset(can_recv_data, 0, sizeof(can_recv_data));
	memset(&RxMessage, 0, sizeof(RxMessage));


	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = FILTERIDHIGH << 5;
	sFilterConfig.FilterIdLow = FILTERIDLOW;
	sFilterConfig.FilterMaskIdHigh = FILTERMASKIDHIGH << 5;
	sFilterConfig.FilterMaskIdLow = FILTERMASKIDLOW;
	sFilterConfig.FilterFIFOAssignment = CAN_USE_FIFO;
	sFilterConfig.FilterActivation = CAN_FILTER_ENABLE;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.SlaveStartFilterBank = 14;

	if(HAL_CAN_ConfigFilter(&CAN_PORT, &sFilterConfig) == HAL_OK)
	{
		#if defined(CAN_MODE_INTERRUPT)
		if(HAL_CAN_ActivateNotification(&CAN_PORT, CAN_IT_RX_FIFO0_MSG_PENDING) == HAL_OK)
		{
			if(HAL_CAN_Start(&CAN_PORT) == HAL_OK)
			{
				status = HAL_OK;
			}
		}
		#endif
		#if defined(CAN_MODE_POLLING)
		if(HAL_CAN_Start(&CAN_PORT) == HAL_OK)
		{
			status = HAL_OK;
		}
		#endif
	}
	return status;
}

void can_task(void)
{


	if(((RxMessage.StdId == 0x120 ||RxMessage.StdId == 0x220||RxMessage.StdId == 0x320||RxMessage.StdId == 0x420)) && (RxMessage.DLC == 8))
	{
		if(RxMessage.StdId == 0x120)
		{
			memset(&buck_boost[0].SC_Voltage_1, 0, 8);
			memcpy(&buck_boost[0].SC_Voltage_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x220)
		{
			memset(&buck_boost[1].SC_Voltage_1, 0, 8);
			memcpy(&buck_boost[1].SC_Voltage_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x320)
		{
			memset(&buck_boost[2].SC_Voltage_1, 0, 8);
			memcpy(&buck_boost[2].SC_Voltage_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x420)
		{
			memset(&buck_boost[3].SC_Voltage_1, 0, 8);
			memcpy(&buck_boost[3].SC_Voltage_1, &can_recv_data[0], 8);
		}


	}
	//else if((RxMessage.StdId == 0x121) && (RxMessage.DLC == 8))
	else if((RxMessage.StdId == 0x121 || RxMessage.StdId == 0x221 || RxMessage.StdId == 0x321 || RxMessage.StdId == 0x421) && (RxMessage.DLC == 8))
	{
		//temp = atoi((char *)can_recv_data);
		if(RxMessage.StdId == 0x121)
		{
			memset(&buck_boost[0].SC_Current_1, 0, 8);
			memcpy(&buck_boost[0].SC_Current_1, &can_recv_data, 8);
		}
		else if(RxMessage.StdId == 0x221)
		{
			memset(&buck_boost[1].SC_Current_1, 0, 8);
			memcpy(&buck_boost[1].SC_Current_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x321)
		{
			memset(&buck_boost[2].SC_Current_1, 0, 8);
			memcpy(&buck_boost[2].SC_Current_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x421)
		{
			memset(&buck_boost[3].SC_Current_1, 0, 8);
			memcpy(&buck_boost[3].SC_Current_1, &can_recv_data[0], 8);
		}

		//char *ptr = (char *)&buck_boost.SC_Voltage_1;


/*		char temp_str[128];
		memset(temp_str, 0, sizeof(temp_str));
		//sprintf(temp_str,"BRA,%d,%d,%d,%d,",buck_boost.SC_Voltage_1,buck_boost.SC_Voltage_2,buck_boost.SC_Voltage_3,buck_boost.SC_Voltage_4);
		memcpy(temp_str, &buck_boost.SC_Current_1, 8);
		printf("%x\r\n", temp_str[0]);
		printf("%x\r\n", temp_str[1]);
		printf("%x\r\n", temp_str[2]);
		printf("%x\r\n", temp_str[3]);
		printf("%x\r\n", temp_str[4]);
		printf("%x\r\n", temp_str[5]);
		printf("%x\r\n", temp_str[6]);
		printf("%x\r\n", temp_str[7]);*/



/*		uint16_t sc_Current1_data = (uint16_t)buck_boost.SC_Current_1;
		printf("Current: %x\n", sc_Current1_data);
		uint16_t sc_Current2_data = (uint16_t)buck_boost.SC_Current_2;
		printf("Current: %x\n", sc_Current2_data);
		uint16_t sc_Current3_data = (uint16_t)buck_boost.SC_Current_3;
		printf("Current: %x\n", sc_Current3_data);
		uint16_t sc_Current4_data = (uint16_t)buck_boost.SC_Current_4;
		printf("Current: %x\n", sc_Current4_data);*/

/*		printf("TP1 = %d\r\n", atoi((char *)buck_boost.SC_Current_1));   //TCP RX
		temp = atoi((char *)can_recv_data);
		printf("TP2 = %d\r\n", temp);*/
		/*
		memset(temp_arry_str, 0, sizeof(temp_arry_str));
		ptr = strtok((char *)can_recv_data, ",");
		memcpy(&temp_arry_str[0][0], ptr, strlen(ptr));
		ptr = strtok(NULL,",");
		memcpy(&temp_arry_str[1][0], ptr, strlen(ptr));
		printf("%s\n", &temp_arry_str[0][0]);
		printf("%s\n", &temp_arry_str[1][0]);

		temp1 = atoi(&temp_arry_str[0][0]);
		temp2 = atoi(&temp_arry_str[1][0]);
		printf("TP3 = %d\r\n",temp1);
		printf("TP4 = %d\r\n",temp2);
		printf("TP5 = %x\r\n",temp1);
		printf("TP6 = %x\r\n",temp2);*/

		//memset(temp_str, 0, sizeof(temp_str));
		//sprintf(temp_str,"BRA,%c,%c,%c,%c",buck_boost.SC_Current_1,buck_boost.SC_Current_2,buck_boost.SC_Current_3,buck_boost.SC_Current_4);
		//printf("%s\n",temp_str);
		//char *ptr = (char *)&buck_boost.SC_Voltage_1;   // ����ü �����ͼ���
	}
	//else if((RxMessage.StdId == 0x122 || RxMessage.StdId == 0x222 || RxMessage.StdId == 0x322 || RxMessage.StdId == 0x422) && (RxMessage.DLC == 8))
	else if(RxMessage.StdId == 0x122 || RxMessage.StdId == 0x222 || RxMessage.StdId == 0x322 || RxMessage.StdId == 0x422)
	{
		if(RxMessage.StdId == 0x122)
		{
			memset(&buck_boost[0].Link_Voltage_1, 0, 8);
			memcpy(&buck_boost[0].Link_Voltage_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x222)
		{
			memset(&buck_boost[1].Link_Voltage_1, 0, 8);
			memcpy(&buck_boost[1].Link_Voltage_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x322)
		{
			memset(&buck_boost[2].Link_Voltage_1, 0, 8);
			memcpy(&buck_boost[2].Link_Voltage_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x422)
		{
			memset(&buck_boost[3].Link_Voltage_1, 0, 8);
			memcpy(&buck_boost[3].Link_Voltage_1, &can_recv_data[0], 8);
		}
	}

	//else if((RxMessage.StdId == 0x123 || RxMessage.StdId == 0x223 || RxMessage.StdId == 0x323 || RxMessage.StdId == 0x423) && (RxMessage.DLC == 8))
	else if(RxMessage.StdId == 0x123 || RxMessage.StdId == 0x223 || RxMessage.StdId == 0x323 || RxMessage.StdId == 0x423)
	{
		if(RxMessage.StdId == 0x123)
		{
			memset(&buck_boost[0].Link_Current_1, 0, 8);
			memcpy(&buck_boost[0].Link_Current_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x223)
		{
			memset(&buck_boost[1].Link_Current_1, 0, 8);
			memcpy(&buck_boost[1].Link_Current_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x323)
		{
			memset(&buck_boost[2].Link_Current_1, 0, 8);
			memcpy(&buck_boost[2].Link_Current_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x423)
		{
			memset(&buck_boost[3].Link_Current_1, 0, 8);
			memcpy(&buck_boost[3].Link_Current_1, &can_recv_data[0], 8);
		}
	}
	//else if((RxMessage.StdId == 0x124 || RxMessage.StdId == 0x224 || RxMessage.StdId == 0x324 || RxMessage.StdId == 0x424) && (RxMessage.DLC == 5))
	else if(RxMessage.StdId == 0x124 || RxMessage.StdId == 0x224 || RxMessage.StdId == 0x324 || RxMessage.StdId == 0x424)
	{
		//if((RxMessage.StdId == 0x124) && (RxMessage.DLC == 5))
		if(RxMessage.StdId == 0x124)
		{
			memset(&buck_boost[0].Temp_1, 0, 8);
			memcpy(&buck_boost[0].Temp_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x224)
		{
			memset(&buck_boost[1].Temp_1, 0, 8);
			memcpy(&buck_boost[1].Temp_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x324)
		{
			memset(&buck_boost[2].Temp_1, 0, 8);
			memcpy(&buck_boost[2].Temp_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x424)
		{
			memset(&buck_boost[3].Temp_1, 0, 8);
			memcpy(&buck_boost[3].Temp_1, &can_recv_data[0], 8);
		}

	}
	//else if((RxMessage.StdId == 0x125 || RxMessage.StdId == 0x225 || RxMessage.StdId == 0x325 || RxMessage.StdId == 0x425) && (RxMessage.DLC == 8))
	else if(RxMessage.StdId == 0x125 || RxMessage.StdId == 0x225 || RxMessage.StdId == 0x325 || RxMessage.StdId == 0x425)
	{
		if(RxMessage.StdId == 0x125)
		{
			memset(&buck_boost[0].SOC_1, 0, 8);
			memcpy(&buck_boost[0].SOC_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x225)
		{
			memset(&buck_boost[1].SOC_1, 0, 8);
			memcpy(&buck_boost[1].SOC_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x325)
		{
			memset(&buck_boost[2].SOC_1, 0, 8);
			memcpy(&buck_boost[2].SOC_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x425)
		{
			memset(&buck_boost[3].SOC_1, 0, 8);
			memcpy(&buck_boost[3].SOC_1, &can_recv_data[0], 8);
		}
	}
	//else if((RxMessage.StdId == 0x126 || RxMessage.StdId == 0x226 || RxMessage.StdId == 0x326 || RxMessage.StdId == 0x426) && (RxMessage.DLC == 8))
	else if(RxMessage.StdId == 0x126 || RxMessage.StdId == 0x226 || RxMessage.StdId == 0x326 || RxMessage.StdId == 0x426)
	{
		if(RxMessage.StdId == 0x126)
		{
			memset(&buck_boost[0].Operation_State_1, 0, 8);
			memcpy(&buck_boost[0].Operation_State_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x226)
		{
			memset(&buck_boost[1].Operation_State_1, 0, 8);
			memcpy(&buck_boost[1].Operation_State_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x326)
		{
			memset(&buck_boost[2].Operation_State_1, 0, 8);
			memcpy(&buck_boost[2].Operation_State_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x426)
		{
			memset(&buck_boost[3].Operation_State_1, 0, 8);
			memcpy(&buck_boost[3].Operation_State_1, &can_recv_data[0], 8);
		}
	}
	//else if((RxMessage.StdId == 0x127 || RxMessage.StdId == 0x227 || RxMessage.StdId == 0x327 || RxMessage.StdId == 0x427) && (RxMessage.DLC == 8))
	else if(RxMessage.StdId == 0x127 || RxMessage.StdId == 0x227 || RxMessage.StdId == 0x327 || RxMessage.StdId == 0x427)
	{
		if(RxMessage.StdId == 0x127)
		{
			memset(&buck_boost[0].Cycle_State_1, 0, 8);
			memcpy(&buck_boost[0].Cycle_State_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x227)
		{
			memset(&buck_boost[1].Cycle_State_1, 0, 8);
			memcpy(&buck_boost[1].Cycle_State_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x327)
		{
			memset(&buck_boost[2].Cycle_State_1, 0, 8);
			memcpy(&buck_boost[2].Cycle_State_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x427)
		{
			memset(&buck_boost[3].Cycle_State_1, 0, 8);
			memcpy(&buck_boost[3].Cycle_State_1, &can_recv_data[0], 8);
		}
	}
	//else if((RxMessage.StdId == 0x128 || RxMessage.StdId == 0x228 || RxMessage.StdId == 0x328 || RxMessage.StdId == 0x428) && (RxMessage.DLC == 8))
	else if(RxMessage.StdId == 0x128 || RxMessage.StdId == 0x228 || RxMessage.StdId == 0x328 || RxMessage.StdId == 0x428)
	{
		if(RxMessage.StdId == 0x128)
		{
			memset(&buck_boost[0].Mode_ID_1, 0, 8);
			memcpy(&buck_boost[0].Mode_ID_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x228)
		{
			memset(&buck_boost[1].Mode_ID_1, 0, 8);
			memcpy(&buck_boost[1].Mode_ID_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x328)
		{
			memset(&buck_boost[2].Mode_ID_1, 0, 8);
			memcpy(&buck_boost[2].Mode_ID_1, &can_recv_data[0], 8);
		}
		else if(RxMessage.StdId == 0x428)
		{
			memset(&buck_boost[3].Mode_ID_1, 0, 8);
			memcpy(&buck_boost[3].Mode_ID_1, &can_recv_data[0], 8);
		}

	}
	else if(RxMessage.StdId == 0x20)
	{
		memset(&acdc.ac_r_voltage, 0, 6);
		memcpy(&acdc.ac_r_voltage, &can_recv_data[0], 6);

	}
	else if(RxMessage.StdId == 0x21)
	{
		memset(&acdc.ac_r_current, 0, 6);
		memcpy(&acdc.ac_r_current, &can_recv_data[0], 6);

	}
	else if(RxMessage.StdId == 0x22)
	{
		memset(&acdc.ac_frequency_r, 0, 8);
		memcpy(&acdc.ac_frequency_r, &can_recv_data[0], 8);

	}
	else if(RxMessage.StdId == 0x23)
	{
		memset(&acdc.apparent_power, 0, 8);
		memcpy(&acdc.apparent_power, &can_recv_data[0], 8);

	}
	else if(RxMessage.StdId == 0x24)
	{
		memset(&acdc.dc_power, 0, 2);
		memcpy(&acdc.dc_power, &can_recv_data[0], 2);

	}
	else if(RxMessage.StdId == 0x25)
	{
/*		int i;
		int position =0;
		memset(&acdc.out_out_relay, 0, 8);
		memcpy(&acdc.out_out_relay, &can_recv_data[0], 8);*/

		char temp_value[8];
		memset(temp_value, 0, sizeof(temp_value));
		memset(&acdc.out_out_relay, 0, 8);
		memset(&acdc.flag1, 0, 8);
		memcpy(&acdc.flag1, &can_recv_data[0], 8);
		memcpy(temp_value, &can_recv_data[0], 8);

		acdc.out_out_relay = (unsigned char)((temp_value[0] >> 0) & 0x01);
		acdc.out_init_relay = (unsigned char)((temp_value[0] >> 1) & 0x01);
		acdc.out_pwm12cs = (unsigned char)((temp_value[0] >> 2) & 0x01);
		acdc.out_pwm34sc = (unsigned char)((temp_value[0] >> 3) & 0x01);
		acdc.out_pwm56sc = (unsigned char)((temp_value[0] >> 4) & 0x01);
		acdc.out_fault_led = (unsigned char)((temp_value[0] >> 5) & 0x01);
		acdc.out_run_led = (unsigned char)((temp_value[0] >> 6) & 0x01);
		acdc.out_ready_led = (unsigned char)((temp_value[0] >> 7) & 0x01);

		acdc.out_fan = (unsigned char)((temp_value[1] >> 0) & 0x01);

		acdc.flag_ac_ready = (unsigned char)((temp_value[2] >> 0) & 0x01);
		acdc.flag_comm_ok = (unsigned char)((temp_value[2] >> 1) & 0x01);
		acdc.flag_dc_ready = (unsigned char)((temp_value[2] >> 2) & 0x01);
		acdc.flag_dclink_ok = (unsigned char)((temp_value[2] >> 3) & 0x01);
		acdc.flag_pll_ok = (unsigned char)((temp_value[2] >> 4) & 0x01);
		acdc.flag_zerocross = (unsigned char)((temp_value[2] >> 5) & 0x01);
		acdc.flag_run = (unsigned char)((temp_value[2] >> 6) & 0x01);
		acdc.flag_stop = (unsigned char)((temp_value[2] >> 7) & 0x01);

		acdc.flag_fault_stop = (unsigned char)((temp_value[3] >> 0) & 0x01);
		acdc.flag_pwm_on = (unsigned char)((temp_value[3] >> 1) & 0x01);
		acdc.flag_pwm_steady = (unsigned char)((temp_value[3] >> 2) & 0x01);
		acdc.flag_dummy = (unsigned char)((temp_value[3] >> 3) & 0x01);
		acdc.flag_pretest = (unsigned char)((temp_value[3] >> 4) & 0x01);
		acdc.flag_testmode = (unsigned char)((temp_value[3] >> 5) & 0x01);
		acdc.flag_gc_mode = (unsigned char)((temp_value[3] >> 6) & 0x01);

		acdc.ovr = (unsigned char)((temp_value[4] >> 0) & 0x01);
		acdc.ovs = (unsigned char)((temp_value[4] >> 1) & 0x01);
		acdc.ovt = (unsigned char)((temp_value[4] >> 2) & 0x01);
		acdc.uvr = (unsigned char)((temp_value[4] >> 3) & 0x01);
		acdc.uvs = (unsigned char)((temp_value[4] >> 4) & 0x01);
		acdc.uvt = (unsigned char)((temp_value[4] >> 5) & 0x01);
		acdc.ocr = (unsigned char)((temp_value[4] >> 6) & 0x01);
		acdc.ocs = (unsigned char)((temp_value[4] >> 7) & 0x01);

		acdc.oct = (unsigned char)((temp_value[5] >> 0) & 0x01);
		acdc.of = (unsigned char)((temp_value[5] >> 1) & 0x01);
		acdc.uf = (unsigned char)((temp_value[5] >> 2) & 0x01);
		acdc.ocdc = (unsigned char)((temp_value[5] >> 3) & 0x01);
		acdc.ovdc = (unsigned char)((temp_value[5] >> 4) & 0x01);
		acdc.uvdc = (unsigned char)((temp_value[5] >> 5) & 0x01);
		acdc.otsen = (unsigned char)((temp_value[5] >> 6) & 0x01);

		acdc.aciint = (unsigned char)((temp_value[6] >> 0) & 0x01);
		acdc.dcvint = (unsigned char)((temp_value[6] >> 1) & 0x01);
		acdc.igbt1 = (unsigned char)((temp_value[6] >> 2) & 0x01);
		acdc.igbt2 = (unsigned char)((temp_value[6] >> 3) & 0x01);
		acdc.igbt3 = (unsigned char)((temp_value[6] >> 4) & 0x01);
		acdc.pwm_trip = (unsigned char)((temp_value[6] >> 5) & 0x01);
		acdc.temp_sw1 = (unsigned char)((temp_value[6] >> 6) & 0x01);
		acdc.temp_sw2 = (unsigned char)((temp_value[6] >> 7) & 0x01);

		acdc.fan1_lock = (unsigned char)((temp_value[7] >> 0) & 0x01);
		acdc.fan2_lock = (unsigned char)((temp_value[7] >> 1) & 0x01);


	}
	else if(RxMessage.StdId == 0x40)
	{
		memset(&psfb.fb_lv_v, 0, 8);
		memcpy(&psfb.fb_lv_v, &can_recv_data[0], 8);

	}
	else if(RxMessage.StdId == 0x41)
		{
			memset(&psfb.fb_fault_1, 0, 8);
			memcpy(&psfb.fb_fault_1, &can_recv_data[0], 4);

		}

}


HAL_StatusTypeDef can_tx(unsigned int IDE, unsigned int RTR, unsigned int StdId, unsigned int ExtId, char *ptr, unsigned int length)
{
	uint8_t tx_buffer[8]={0};
	uint32_t TxMailbox_Check=0;
	CAN_TxHeaderTypeDef TxMessage;
	HAL_StatusTypeDef result = HAL_ERROR;

	if(length > 8)
	{
		printf("Use length less than 8bytes\r\n");
		length = 8;
	}
	memset(tx_buffer, 0, sizeof(tx_buffer));
	TxMessage.IDE = IDE;
	TxMessage.RTR = RTR;
	TxMessage.DLC = length;
	TxMessage.StdId = StdId;
	TxMessage.ExtId = ExtId;
	memcpy(tx_buffer, ptr, length);

	TxMailbox_Check = HAL_CAN_GetTxMailboxesFreeLevel(&CAN_PORT);
	if(TxMailbox_Check)
	{
		if(HAL_CAN_AddTxMessage(&CAN_PORT, &TxMessage, tx_buffer, &TxMailbox_Check) == HAL_OK)
		{
			result = HAL_OK;
		}
	}
	else result = HAL_BUSY;
	/*while(1)
	{
		TxMailbox_Check = HAL_CAN_GetTxMailboxesFreeLevel(&CAN_PORT);
		if(TxMailbox_Check)
		{
			if(HAL_CAN_AddTxMessage(&hcan1, &TxMessage, tx_buffer, &TxMailbox_Check) == HAL_OK)
			{
				printf("CAN TX SUCCESS\r\n");
				break;
			}
			else
			{
				printf("CAN TX FAIL\r\n");
				break;
			}
		}
	}*/
	return result;
}
#if defined(CAN_MODE_INTERRUPT)
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

	if(hcan->Instance == CAN_PORT.Instance)
	{

  	  if(HAL_CAN_GetRxMessage(hcan, CAN_USE_FIFO, &RxMessage, can_recv_data) == HAL_OK)
  	  {
  		  can_flag=1;
  		  can_task();
  	  }
  	  //else printf("GetRxMessage Fail\r\n");
	}
	//else printf("CAN_PORT MISMATCH\r\n");
	can_flag=0;
}
#endif
#endif
