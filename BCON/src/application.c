/*
 * application.c
 *
 *  Created on: 2019. 6. 24.
 *      Author: ghkim
 */

#include "application.h"
uint8_t bprf=0;
#define Bprintf if(bprf) printf
extern IWDG_HandleTypeDef hiwdg;

void ep496_init(void)
{
/*	FLASH_If_Init();
	bist_offset = (int)(*(uint32_t *)OFFSET_ADDRESS);
	//BCON Output Disable
		//bcon_output_enable(0);
	bcon_output_enable(((long long) 0x0000000000000000));
	//BCON BIST Disable
	//bist_output_enable(0);
	bist_output_enable(((long long) 0x0000000000000000));
	//BCON Mode Value Init : NONE, FORWARD, 3D, OUT ALL OFF,BOTTOM, LED OFF
	//bcon_output_enable_data = 0x00000000;
	bcon_output_enable_data = 0x0000000000000000;
	bcon_mode_osg_data = NONE;
	bcon_mode_direction = FORWARD;
	//bcon_mode_3d_6d = _3D;
	//bcon_mode_el_swap = BOTTOM;
	bcon_led_on_off = LED_ON;
	//bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_mode_3d_6d, bcon_mode_el_swap, bcon_led_on_off);
	bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_led_on_off, bcon_mode_output_enable_mask);
	memset(&model_config, 0, sizeof(model_config));
	//BIST Mode Init : ALL Not use, ALL ADC, Delay 0us
	//bist_mode_use = 0x00000000;
	//bist_mode_adc_dac = 0x00000000;
	bist_mode_use = 0x0000000000000000;
	//bist_mode_adc_dac = 0x0000000000000000;
	memset(&bist_config, 0, sizeof(bist_config));
	//BIST ADC Init
	while(HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK);
	//BIST ADC Value Init
	bist_flag = 0;
	bist_cnt = 0;
	memset(bist_adc_value, 0, sizeof(bist_adc_value));
	//BCON ID Read
	bcon_id = bcon_id_read();
	printf("MY BCON BASE ID = 0x%02x\r\n",BCON_BASE_ID);
	printf("MY BCON ID = 0x%02x\r\n",BCON_ID);
	printf("EP496 INIT OK\r\n");*/

	fw_dw_operation = 0;
	FLASH_If_Init();
	memset(&bist_cal, 0, sizeof(bist_cal));
	memcpy(&bist_cal, (uint32_t *)OFFSET_ADDRESS, sizeof(bist_cal));
	//bist_offset = (int)(*(uint32_t *)OFFSET_ADDRESS);
	//z_offset = (int)(*(uint32_t *)OFFSET_ADDRESS_2);
	bcon_output_enable(((long long) 0x0000000000000000));
	//bist_output_enable(((long long) 0x0000000000000000));
	//BCON Mode Value Init : NONE, FORWARD, ALL OFF , LED ON
	bcon_output_enable_data = 0x0000000000000000;
	bcon_mode_osg_data = NONE;
	bcon_mode_direction = FORWARD;
	bcon_led_on_off = LED_ON;
	bcon_mode_output_enable_mask = (long long) 0x0000000000000000;
	bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_led_on_off, bcon_mode_output_enable_mask);
	memset(&model_config, 0, sizeof(model_config));
	bist_mode_use = 0x0000000000000000;
	memset(&bist_config, 0, sizeof(bist_config));
	//BIST ADC Init
	//while(HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK);
	//BIST ADC Value Init
	bist_flag = 0;
	bist_flag2 = 0;
	bist_cnt = 0;
	fw_down_count = 0;
	memset(bist_adc_value, 0, sizeof(bist_adc_value));
	//BCON ID Read
	bcon_id = bcon_id_read();
	printf("MY BCON BASE ID = 0x%02x\r\n",BCON_BASE_ID);
	printf("MY BCON ID = 0x%02x\r\n",BCON_ID);
	printf("bist_0v_value = %f\r\n", bist_cal.bist_0v_value);
	printf("bist_0v_step = %f\r\n", bist_cal.bist_0v_step);
	printf("bist_15v_value = %f\r\n", bist_cal.bist_15v_value);
	printf("bist_15v_step = %f\r\n", bist_cal.bist_15v_step);
	printf("bist_ratio = %f\r\n", bist_cal.bist_ratio);
	printf("bist_offset = %f\r\n", bist_cal.bist_offset);
	printf("bist_user_offset = %f\r\n", bist_cal.bist_user_offset);

	printf("EP496 INIT OK\r\n");
}

uint16_t bcon_id_read(void)
{
	uint16_t id=0x0000;

	/*id = (HAL_GPIO_ReadPin(ID0_GPIO_Port, ID0_Pin) << 8)
		+ (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID1_Pin) << 9)
		+ (HAL_GPIO_ReadPin(ID2_GPIO_Port, ID2_Pin) << 10)
		+ (HAL_GPIO_ReadPin(ID3_GPIO_Port, ID3_Pin) << 11)
		+ (HAL_GPIO_ReadPin(ID4_GPIO_Port, ID4_Pin) << 0)
		+ (HAL_GPIO_ReadPin(ID5_GPIO_Port, ID5_Pin) << 1)
		+ (HAL_GPIO_ReadPin(ID6_GPIO_Port, ID6_Pin) << 2);*/
	id = (HAL_GPIO_ReadPin(ID5_GPIO_Port, ID5_Pin) << 10)
	  +	 (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID4_Pin) << 9)
	  +	 (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID3_Pin) << 8)
	  +	 (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID2_Pin) << 2)
	  +	 (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID1_Pin) << 1)
	  +	 (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID0_Pin) << 0);

	return id;
}

//void bcon_output_enable(uint32_t data_30bit)
void bcon_output_enable(long long data_64bit)
{
	//uint32_t temp_data_30bit = 0x00000000;
	long long temp_data_64bit = (long long) 0x0000000000000000;
	//long long temp_data_test = 0x0000000000000000;

	//temp_data_test = data_64bit;
	//uint16_t d1_d16;
	//uint16_t d17_d30;
	uint8_t d1_d8;
	uint8_t d9_d16;
	uint8_t d17_d24;
	uint8_t d25_d32;
	uint8_t d33_d40;
	uint8_t d41_d42;
	int i;

	//if((bcon_mode_osg_data == DATA) || (bcon_mode_osg_data == OSG) || (bcon_mode_osg_data == OSG_VT) || (bcon_mode_osg_data == SSD_DATA))
	if((bcon_mode_osg_data == DATA_OSG) || (bcon_mode_osg_data == OSG_12CLK) || (bcon_mode_osg_data == OSG_8CLK) ||(bcon_mode_osg_data == OSG_6CLK) || (bcon_mode_osg_data == DATA_SSD)|| (bcon_mode_osg_data == DATA_77)|| (bcon_mode_osg_data == DATA_NEW))
	{
		if(bcon_mode_direction == REVERSE)//Reverse
		{
			//for(i=0; i<30; i++)
			for(i=0; i<42; i++)
			{
				/*temp_data_30bit = temp_data_30bit | ((data_30bit >> i) & 0x00000001);
				if(i == 29) break;
				temp_data_30bit = (temp_data_30bit << 1);*/
				temp_data_64bit = temp_data_64bit | ((data_64bit >> i) & (long long)0x0000000000000001);
				if(i == 41) break;
				//temp_data_30bit = (temp_data_30bit << 1);
				temp_data_64bit = (temp_data_64bit << 1);
			}
		}
		else if(bcon_mode_direction == FORWARD)//Forward
		{
			//temp_data_30bit = data_30bit;
			temp_data_64bit = data_64bit;
		}
		else
		{
			//temp_data_30bit = 0x00000000;
			temp_data_64bit = (long long) 0x0000000000000000;
			Bprintf("BCON DIRECTION DATA INVALID\r\n");
		}
	}
	else
	{
		//temp_data_30bit = 0x00000000;
		temp_data_64bit = (long long) 0x0000000000000000;
		Bprintf("BCON TYPE IS NONE\r\n");
	}

	//d1_d16 = (uint16_t)(temp_data_30bit & 0x0000ffff);
	//d17_d30 = (uint16_t)((temp_data_30bit >> 16) & 0x0000ffff);
/*	d1_d8 = (uint8_t)(temp_data_64bit & 0xff);
	d9_d16 = (uint8_t)((temp_data_64bit >> 8) & 0xff);
	d17_d24 = (uint8_t)((temp_data_64bit >> 16) & 0xff);
	d25_d32 = (uint8_t)((temp_data_64bit >> 24) & 0xff);
	d33_d40 = (uint8_t)((temp_data_64bit >> 32) & 0xff);
	d41_d42 = (uint8_t)((temp_data_64bit >> 40) & 0x02);*/

	d1_d8 = (uint8_t)(temp_data_64bit & 0xff);
	d9_d16 = (uint8_t)((temp_data_64bit >> 8) & 0xff);
	d17_d24 = (uint8_t)((temp_data_64bit >> 16) & 0xff);
	d25_d32 = (uint8_t)((temp_data_64bit >> 24) & 0xff);
	d33_d40 = (uint8_t)((temp_data_64bit >> 32) & 0xff);
	d41_d42 = (uint8_t)((temp_data_64bit >> 40) & 0x03);

	Bprintf("d1_d8 = %02x\r\n", d1_d8);
	Bprintf("d9_d16 = %02x\r\n", d9_d16);
	Bprintf("d17_d24 = %02x\r\n", d17_d24);
	Bprintf("d25_d32 = %02x\r\n", d25_d32);
	Bprintf("d33_d40 = %02x\r\n", d33_d40);
	Bprintf("d41_d42 = %02x\r\n", d41_d42);
	//D1~D30 Setting
	/*HAL_GPIO_WritePin(D1_D16_GPIO_Port, (~d1_d16) & D1_D16_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D17_D30_GPIO_Port, (~d17_d30) & D17_D30_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D16_GPIO_Port, d1_d16 & D1_D16_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(D17_D30_GPIO_Port, d17_d30 & D17_D30_Pin_MASK, GPIO_PIN_SET);
	//OUTPUT_CLK : RISING EDGE
	HAL_GPIO_WritePin(OUTPUT_CLK_GPIO_Port, OUTPUT_C.LK_Pin, GPIO_PIN_RESET);
	while(HAL_GPIO_ReadPin(OUTPUT_CLK_GPIO_Port, OUTPUT_CLK_Pin) != GPIO_PIN_RESET);
	HAL_GPIO_WritePin(OUTPUT_CLK_GPIO_Port, OUTPUT_CLK_Pin, GPIO_PIN_SET);
	Bprintf("BCON OUTPUT ENABLE DATA = 0x%04x%04x\r\n", d17_d30, d1_d16);*/

	//D1~D42 Setting
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (uint16_t)((~d1_d8) & D1_D8_Pin_MASK), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (uint16_t)(d1_d8 & D1_D8_Pin_MASK), GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK1_GPIO_Port, CLK1_Pin, GPIO_PIN_RESET);
	if(io_check(CLK1_GPIO_Port,CLK1_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK1_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK1_GPIO_Port, CLK1_Pin, GPIO_PIN_SET);
	if(io_check(CLK1_GPIO_Port,CLK1_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK1_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d9_d16) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d9_d16 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK2_GPIO_Port, CLK2_Pin, GPIO_PIN_RESET);
	if(io_check(CLK2_GPIO_Port,CLK2_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK2_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK2_GPIO_Port, CLK2_Pin, GPIO_PIN_SET);
	if(io_check(CLK2_GPIO_Port,CLK2_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK2_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d17_d24) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d17_d24 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK3_GPIO_Port, CLK3_Pin, GPIO_PIN_RESET);
	if(io_check(CLK3_GPIO_Port,CLK3_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK3_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK3_GPIO_Port, CLK3_Pin, GPIO_PIN_SET);
	if(io_check(CLK3_GPIO_Port,CLK3_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK3_Pin_SET_fail\r\n");
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d25_d32) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d25_d32 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK4_GPIO_Port, CLK4_Pin, GPIO_PIN_RESET);
	if(io_check(CLK4_GPIO_Port,CLK4_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK4_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK4_GPIO_Port, CLK4_Pin, GPIO_PIN_SET);
	if(io_check(CLK4_GPIO_Port,CLK4_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK4_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d33_d40) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d33_d40 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK5_GPIO_Port, CLK5_Pin, GPIO_PIN_RESET);
	if(io_check(CLK5_GPIO_Port,CLK5_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK5_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK5_GPIO_Port, CLK5_Pin, GPIO_PIN_SET);
	if(io_check(CLK5_GPIO_Port,CLK5_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK5_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d41_d42) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d41_d42 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_RESET);
	if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK6_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_SET);
	if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK6_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(OUTPUT_CLK_GPIO_Port, OUTPUT_CLK_Pin, GPIO_PIN_RESET);
	if(io_check(OUTPUT_CLK_GPIO_Port,OUTPUT_CLK_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("BIST_CLK_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(OUTPUT_CLK_GPIO_Port, OUTPUT_CLK_Pin, GPIO_PIN_SET);
}

//void bcon_mode_select(uint8_t osg_data, uint8_t direction, uint8_t _3d_6d, uint8_t el_swap, uint8_t led_on_off)
//{
void bcon_mode_select(uint8_t osg_data, uint8_t direction, uint8_t led_on_off, long long on_off_mask)
{
	/*if(_3d_6d == _3D)
	{
		//3D Select
		HAL_GPIO_WritePin(SEL_3D_6D_GPIO_Port, SEL_3D_6D_Pin, GPIO_PIN_RESET);
		Bprintf("3D_6D SELECT = 3D\r\n");
	}
	else if(_3d_6d == _6D)
	{
		//6D Select
		HAL_GPIO_WritePin(SEL_3D_6D_GPIO_Port, SEL_3D_6D_Pin, GPIO_PIN_SET);
		Bprintf("3D_6D SELECT = 6D\r\n");
	}
	else
	{
		Bprintf("3D_6D SELECT INVALID\r\n");
	}*/

	if(osg_data == NONE)
	{
		//Select NONE
		bcon_output_enable_data = OUTPUT_ENABLE_NONE & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_RESET);
		Bprintf("TYPE SELECT = NONE\r\n");
	}
	else if(osg_data == DATA_OSG)
	{
		//Select DATA
		bcon_output_enable_data = OUTPUT_ENABLE_DATA_OSG & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_RESET);
		Bprintf("TYPE SELECT = DATA\r\n");
	}
	else if(osg_data == OSG_12CLK)
	{
		//Select OSG_ACT
		bcon_output_enable_data = OUTPUT_ENABLE_OSG_12CLK & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_SET);
		Bprintf("TYPE SELECT = OSG_ACT\r\n");
	}
	else if(osg_data == DATA_SSD)
	{
		if(ssd_data_cal_flag)
		{
			//Select OSG_ACT
			bcon_output_enable_data = OUTPUT_ENABLE_DATA_SSD_CAL & on_off_mask;
			HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_SET);
			Bprintf("TYPE SELECT = DATA_SSD\r\n");
		}
		else
		{
			//Select OSG_ACT
			bcon_output_enable_data = OUTPUT_ENABLE_DATA_SSD & on_off_mask;
			HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_SET);
			Bprintf("TYPE SELECT = DATA_SSD\r\n");
		}
	}
	else if(osg_data == OSG_8CLK)
	{
		//Select OSG_ACT
		bcon_output_enable_data = OUTPUT_ENABLE_OSG_8CLK & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_SET);
		Bprintf("TYPE SELECT = OSG_ACT\r\n");
	}
	else if(osg_data == OSG_6CLK)
	{
		//Select OSG_ACT
		bcon_output_enable_data = OUTPUT_ENABLE_OSG_6CLK & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_SET);
		Bprintf("TYPE SELECT = OSG_ACT\r\n");
	}
	else if(osg_data == DATA_77)
	{
		bcon_output_enable_data = OUTPUT_ENABLE_DATA_77 & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_RESET);
		Bprintf("TYPE SELECT = DATA_77\r\n");
	}
	else if(osg_data == DATA_NEW)
	{
		bcon_output_enable_data = OUTPUT_ENABLE_DATA_NEW & on_off_mask;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_B_Pin, GPIO_PIN_RESET);
		Bprintf("TYPE SELECT = DATA_NEW\r\n");
	}
	/*else if(osg_data == OSG_VT)
	{
		//Select OSG_VT
		bcon_output_enable_data = OUTPUT_ENABLE_OSG_VT;
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_Pin, GPIO_PIN_SET);
		Bprintf("TYPE SELECT = OSG_VT\r\n");
	}*/
	else
	{
		Bprintf("TYPE SELECT INVALID\r\n");
	}

	if(direction == FORWARD)
	{
		//Select Forward
		HAL_GPIO_WritePin(SEL_DIRECTION_GPIO_Port, SEL_DIRECTION_Pin, GPIO_PIN_RESET);
		Bprintf("FORWARD/REVERSE SELECT = FORWARD\r\n");
	}
	else if(direction == REVERSE)
	{
		//Select Reverse
		HAL_GPIO_WritePin(SEL_DIRECTION_GPIO_Port, SEL_DIRECTION_Pin, GPIO_PIN_SET);
		Bprintf("FORWARD/REVERSE SELECT = REVERSE\r\n");
	}
	else
	{
		Bprintf("FORWARD/REVERSE SELECT INBALID\r\n");
	}

	/*if(el_swap == BOTTOM)
	{
		//Select BOTTOM
		HAL_GPIO_WritePin(SEL_TOP_BOT_GPIO_Port, SEL_TOP_BOT_Pin, GPIO_PIN_RESET);
		Bprintf("TOP/BOTTOM SELECT = BOTTOM\r\n");
	}
	else if(el_swap == TOP)
	{
		//Select TOP
		HAL_GPIO_WritePin(SEL_OSG_DATA_GPIO_Port, SEL_OSG_DATA_Pin, GPIO_PIN_RESET); // DATA Forced Select
		Bprintf("DATA SELECT FOR TOP\r\n");
		bcon_output_enable_data = OUTPUT_ENABLE_TOP;
		HAL_GPIO_WritePin(SEL_TOP_BOT_GPIO_Port, SEL_TOP_BOT_Pin, GPIO_PIN_SET);
		Bprintf("TOP/BOTTOM SELECT = TOP\r\n");
	}
	else
	{
		Bprintf("TOP/BOTTOM SELECT INVALID\r\n");
	}*/

	if(led_on_off == LED_OFF)
	{
		//Select LED OFF
		HAL_GPIO_WritePin(SEL_LED_ON_OFF_GPIO_Port, SEL_LED_ON_OFF_Pin, GPIO_PIN_RESET);
		Bprintf("LED ON/OFF SELECT = LED OFF\r\n");
	}
	else if(led_on_off == LED_ON)
	{
		//Select LED ON
		HAL_GPIO_WritePin(SEL_LED_ON_OFF_GPIO_Port, SEL_LED_ON_OFF_Pin, GPIO_PIN_SET);
		Bprintf("LED ON/OFF SELECT = LED ON\r\n");
	}
	else
	{
		Bprintf("LED ON/OFF SELECT INVALID\r\n");
	}
}

//void bist_output_enable(uint32_t data_30bit)
void bist_output_enable(long long data_64bit)
{
	/*uint16_t d1_d16 = (uint16_t)(data_30bit & 0x0000ffff);
	uint16_t d17_d30 = (uint16_t)((data_30bit >> 16) & 0x0000ffff);

	//D1~D30 Setting
	HAL_GPIO_WritePin(D1_D16_GPIO_Port, (~d1_d16) & D1_D16_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D17_D30_GPIO_Port, (~d17_d30) & D17_D30_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D16_GPIO_Port, d1_d16 & D1_D16_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(D17_D30_GPIO_Port, d17_d30 & D17_D30_Pin_MASK, GPIO_PIN_SET);
	//BIST_CLK : RISING EDGE
	HAL_GPIO_WritePin(BIST_CLK_GPIO_Port, BIST_CLK_Pin, GPIO_PIN_RESET);
	while(HAL_GPIO_ReadPin(BIST_CLK_GPIO_Port, BIST_CLK_Pin) != GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BIST_CLK_GPIO_Port, BIST_CLK_Pin, GPIO_PIN_SET);
	//Bprintf("BIST OUTPUT ENABLE DATA = 0x%04x%04x\r\n", d17_d30, d1_d16);*/

	uint8_t d1_d8;
	uint8_t d9_d16;
	uint8_t d17_d24;
	uint8_t d25_d32;
	uint8_t d33_d40;
	uint8_t d41_d42;

	d1_d8 = (uint8_t)(data_64bit & 0xff);
	d9_d16 = (uint8_t)((data_64bit >> 8) & 0xff);
	d17_d24 = (uint8_t)((data_64bit >> 16) & 0xff);
	d25_d32 = (uint8_t)((data_64bit >> 24) & 0xff);
	d33_d40 = (uint8_t)((data_64bit >> 32) & 0xff);
	d41_d42 = (uint8_t)((data_64bit >> 40) & 0x03);

/*	Bprintf("d1_d8 = %02x\r\n", d1_d8);
	Bprintf("d9_d16 = %02x\r\n", d9_d16);
	Bprintf("d17_d24 = %02x\r\n", d17_d24);
	Bprintf("d25_d32 = %02x\r\n", d25_d32);
	Bprintf("d33_d40 = %02x\r\n", d33_d40);
	Bprintf("d41_d42 = %02x\r\n", d41_d42);*/

	//D1~D42 Setting
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d1_d8) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d1_d8 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK1_GPIO_Port, CLK1_Pin, GPIO_PIN_RESET);
	if(io_check(CLK1_GPIO_Port,CLK1_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK1_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK1_GPIO_Port, CLK1_Pin, GPIO_PIN_SET);
	if(io_check(CLK1_GPIO_Port,CLK1_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK1_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d9_d16) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d9_d16 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK2_GPIO_Port, CLK2_Pin, GPIO_PIN_RESET);
	if(io_check(CLK2_GPIO_Port,CLK2_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK2_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK2_GPIO_Port, CLK2_Pin, GPIO_PIN_SET);
	if(io_check(CLK2_GPIO_Port,CLK2_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK2_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d17_d24) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d17_d24 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK3_GPIO_Port, CLK3_Pin, GPIO_PIN_RESET);
	if(io_check(CLK3_GPIO_Port,CLK3_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK3_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK3_GPIO_Port, CLK3_Pin, GPIO_PIN_SET);
	if(io_check(CLK3_GPIO_Port,CLK3_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK3_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d25_d32) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d25_d32 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK4_GPIO_Port, CLK4_Pin, GPIO_PIN_RESET);
	if(io_check(CLK4_GPIO_Port,CLK4_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK4_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK4_GPIO_Port, CLK4_Pin, GPIO_PIN_SET);
	if(io_check(CLK4_GPIO_Port,CLK4_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK4_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d33_d40) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d33_d40 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK5_GPIO_Port, CLK5_Pin, GPIO_PIN_RESET);
	if(io_check(CLK5_GPIO_Port,CLK5_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK5_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK5_GPIO_Port, CLK5_Pin, GPIO_PIN_SET);
	if(io_check(CLK5_GPIO_Port,CLK5_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK5_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(D1_D8_GPIO_Port, (~d41_d42) & D1_D8_Pin_MASK, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(D1_D8_GPIO_Port, d41_d42 & D1_D8_Pin_MASK, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_RESET);
	if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("CLK6_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(CLK6_GPIO_Port, CLK6_Pin, GPIO_PIN_SET);
	if(io_check(CLK6_GPIO_Port,CLK6_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("CLK6_Pin_SET_fail\r\n");

	HAL_GPIO_WritePin(BIST_CLK_GPIO_Port, BIST_CLK_Pin, GPIO_PIN_RESET);
	if(io_check(BIST_CLK_GPIO_Port,BIST_CLK_Pin,GPIO_PIN_RESET) == HAL_ERROR) printf("BIST_CLK_Pin_RESET_fail\r\n");
	HAL_GPIO_WritePin(BIST_CLK_GPIO_Port, BIST_CLK_Pin, GPIO_PIN_SET);
	if(io_check(BIST_CLK_GPIO_Port,BIST_CLK_Pin,GPIO_PIN_SET) == HAL_ERROR) printf("BIST_CLK_Pin_SET_fail\r\n");
}

HAL_StatusTypeDef io_check(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
	int ret = 0;
	int timeout =0;
	while(1)
	{
		if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin) == PinState) break;
		if(timeout++ >10000)
		{
			ret = -1;
			break;
		}
	}
	if(ret == 0)
	{
		ret= HAL_OK;
	}
	else ret = HAL_ERROR;

	return ret;
}


