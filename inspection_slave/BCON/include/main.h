/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ep.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SEL_OSG_DATA_B_Pin GPIO_PIN_13
#define SEL_OSG_DATA_GPIO_Port GPIOC
#define SEL_OSG_DATA_A_Pin GPIO_PIN_14
#define SEL_OSG_DATA_GPIO_Port GPIOC
#define SEL_DIRECTION_Pin GPIO_PIN_15
#define SEL_DIRECTION_GPIO_Port GPIOC
#define SEL_LED_ON_OFF_Pin GPIO_PIN_1
#define SEL_LED_ON_OFF_GPIO_Port GPIOF
#define ID0_Pin GPIO_PIN_4
#define ID0_GPIO_Port GPIOA
#define ID1_Pin GPIO_PIN_5
#define ID1_GPIO_Port GPIOA
#define ID2_Pin GPIO_PIN_6
#define ID2_GPIO_Port GPIOA
#define ID3_Pin GPIO_PIN_7
#define ID3_GPIO_Port GPIOA
#define D1_Pin GPIO_PIN_0
#define D1_GPIO_Port GPIOB
#define D2_Pin GPIO_PIN_1
#define D2_GPIO_Port GPIOB
#define D3_Pin GPIO_PIN_2
#define D3_GPIO_Port GPIOB
#define CLK1_Pin GPIO_PIN_12
#define CLK1_GPIO_Port GPIOB
#define CLK2_Pin GPIO_PIN_13
#define CLK2_GPIO_Port GPIOB
#define CLK3_Pin GPIO_PIN_14
#define CLK3_GPIO_Port GPIOB
#define CLK4_Pin GPIO_PIN_15
#define CLK4_GPIO_Port GPIOB
#define CLK5_Pin GPIO_PIN_6
#define CLK5_GPIO_Port GPIOC
#define CLK6_Pin GPIO_PIN_7
#define CLK6_GPIO_Port GPIOC
#define ID5_Pin GPIO_PIN_9
#define ID5_GPIO_Port GPIOC
#define ID4_Pin GPIO_PIN_8
#define ID4_GPIO_Port GPIOA
#define BIST_CLK_Pin GPIO_PIN_14
#define BIST_CLK_GPIO_Port GPIOA
#define OUTPUT_CLK_Pin GPIO_PIN_15
#define OUTPUT_CLK_GPIO_Port GPIOA
#define RUN_LED_Pin GPIO_PIN_2
#define RUN_LED_GPIO_Port GPIOD
#define D4_Pin GPIO_PIN_3
#define D4_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_4
#define D5_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_5
#define D6_GPIO_Port GPIOB
#define D7_Pin GPIO_PIN_6
#define D7_GPIO_Port GPIOB
#define D8_Pin GPIO_PIN_7
#define D8_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/*#define D1_D16_GPIO_Port GPIOB
#define D1_D16_Pin_MASK ((uint16_t)0xFFFFU)*/

#define D1_D8_GPIO_Port GPIOB
#define D1_D8_Pin_MASK ((uint8_t)0xFFU)
#define D17_D30_GPIO_Port GPIOC
#define D17_D30_Pin_MASK ((uint16_t)0x3FFFU)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
