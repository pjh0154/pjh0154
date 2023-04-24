/*
 * timer.c
 *
 *  Created on: 2019. 4. 25.
 *      Author: ghkim
 */
#include "timer.h"

#ifdef TIMER1_MODE_INTERRUPT
uint8_t timer1_channel_flag = 0;
uint32_t count = 0;
uint32_t count2 = 0;
//uint32_t i,j;
long long i;
long long ii;
uint32_t j;
uint32_t jj;
//long long ii;
extern uint8_t bprf;
uint8_t aprf=0;
#define Bprintf if(aprf) printf
void timer1_task(void);
extern TIM_HandleTypeDef htim1;
extern IWDG_HandleTypeDef hiwdg;
double adc_value;
double adc_jig_value;
uint32_t adc_value_2;
//unsigned char temp_bist_value[30] = {0};
unsigned char temp_bist_value[42] = {0};
int k=0;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

uint8_t bcon_mode_direction_backup;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIMER1_CHANNEL.Instance)
	{
		timer1_channel_flag = 1;
	}
}

void timer1_task(void)
{
	if(timer1_channel_flag)
	{
		timer1_channel_flag = 0;
		if(count >= 500)
		{
			count = 0;
			HAL_IWDG_Refresh(&hiwdg);
			HAL_GPIO_TogglePin(RUN_LED_GPIO_Port, RUN_LED_Pin);

		}
		else count++;

		if(bist_flag)
		{
			if (bist_cnt == 0)
			{
				//bcon_mode_direction_backup = bcon_mode_direction;
				//bcon_mode_direction = FORWARD;
				//BIST ADC Value Init
				memset(bist_adc_value, 0, sizeof(bist_adc_value));
				//BIST Configure Value Setting
				//bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_mode_3d_6d, bcon_mode_el_swap, bcon_led_on_off);
				bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_led_on_off, bcon_mode_output_enable_mask);
				//bcon_output_enable(bcon_output_enable_data);
				bcon_output_enable(bist_mode_use);
				//i = 0x00000001;
				i = (long long)0x0000000000000001;
				j = 0;
			}
			//else if(bist_cnt == 5)
			else if(bist_cnt == 5)
			{
				bist_output_enable(i);
			}
			else if(bist_cnt == 8)
			{
				HAL_Delay(10);
				//if(i < 0x40000000)
				if(i < (long long)0x40000000000)
				{
					// KGH FIX START
					while(HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK);
					// KGH FIX END
					if(HAL_ADC_Start(&hadc) == HAL_OK)
					{
						if(HAL_ADC_PollForConversion(&hadc,1000) == HAL_OK)
						{
							adc_value = ((((bist_cal.bist_ratio * ((double)HAL_ADC_GetValue(&hadc))) + bist_cal.bist_offset) + 0.05+ bist_cal.bist_user_offset) * 10.0);

							if(adc_value >= 255) adc_value = 255;
							else if(adc_value < 0) adc_value = 0;
							bist_adc_value[j] = (unsigned char)adc_value;
							if(aprf) printf("[CH%d] = %d, [VOLT] = %d\r\n",(int)j, (int)HAL_ADC_GetValue(&hadc), bist_adc_value[j]);
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
					j++;
				}
				i = (i<<1);
				//if(j == 30)
				if(j == 42)
				{
					if(bprf) printf("BIST Operation Done\r\n");
					if(bcon_mode_direction == REVERSE)
					//if(0)
					{
/*						for(k=0; k<30; k++)
						{
							temp_bist_value[k] = bist_adc_value[29-k];
						}
						memcpy(bist_adc_value, &temp_bist_value[0], sizeof(temp_bist_value));*/
						for(k=0; k<42; k++)
						{
							temp_bist_value[k] = bist_adc_value[41-k];
						}
						memcpy(bist_adc_value, &temp_bist_value[0], sizeof(temp_bist_value));
					}
					//BIST Output ALL OFF
					bist_output_enable(0);
					//BCON Output ALL OFF
					bcon_output_enable(0);
					bist_flag = 0;
					bist_cnt = 0;
					if(bprf) printf("BIST RESULT VALUE\r\n");
					for(int i=0; i<42; i++)
					{
						if(bprf) printf("[CH%02d] = %d\r\n",i+1,bist_adc_value[i]);
					}
					__HAL_UART_FLUSH_DRREGISTER(&DEBUG_UART_PORT);
					//bcon_mode_direction = bcon_mode_direction_backup;
					//bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_led_on_off, bcon_mode_output_enable_mask);
				}
				else
				{
					//bist_output_enable(0);
					bist_cnt = 4;
				}
			}
			if(bist_flag == 1) bist_cnt++;
		}


		if(bist_flag2)
		{
			if(bist_cnt_1 == 0)
			{
				//BIST ADC Value Init
				//BIST Configure Value Setting
				//bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_mode_3d_6d, bcon_mode_el_swap, bcon_led_on_off);
				//bcon_mode_select(bcon_mode_osg_data, bcon_mode_direction, bcon_led_on_off, bcon_mode_output_enable_mask);
				//bcon_output_enable(bcon_output_enable_data);

				//i = 0x00000001;
				ii = (long long)0x0000000000000001;
				jj = 0;
			}
			else if(bist_cnt_1 == 5)
			{
				bist_output_enable(i);
			}
			else if(bist_cnt_1 == 8)
			{
				//if(i < 0x40000000)
				if(ii < (long long)0x40000000000)
				{
					if((bist_mode_use >> jj) & (long long)0x0000000000000001)
					{
						if(HAL_ADC_Start(&hadc) == HAL_OK)
						{
							if(HAL_ADC_PollForConversion(&hadc,1000) == HAL_OK)
							{
								//adc_value = ((((double)HAL_ADC_GetValue(&hadc)) - 462) * 0.1018507704) + bist_offset/10;
								adc_value_2 = (uint32_t)HAL_ADC_GetValue(&hadc);
								printf("ADC_VALUE[%d] : %d\r\n", (int)j,(int)adc_value_2);


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
					j++;
				}
				ii = (ii<<1);
				//if(j == 30)
				if(jj == 42)
				{
					if(bprf) printf("BIST Operation Done\r\n");
					//BIST Output ALL OFF
					bist_output_enable(0);
					//BCON Output ALL OFF
					bcon_output_enable(0);
					bist_flag2 = 0;
					bist_cnt_1 = 0;
					if(bprf) printf("BIST RESULT VALUE\r\n");

				}
				else
				{
					//bist_output_enable(0);
					bist_cnt_1 = 4;
				}
			}
			if(bist_flag2 == 1) bist_cnt_1++;
		}
	}
}
#endif
