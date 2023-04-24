/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f2xx_hal.h"

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
#define FAN_ONOFF_Pin GPIO_PIN_2
#define FAN_ONOFF_GPIO_Port GPIOE
#define EMS_IN_Pin GPIO_PIN_6
#define EMS_IN_GPIO_Port GPIOE
#define MC_ONOFF_Pin GPIO_PIN_6
#define MC_ONOFF_GPIO_Port GPIOI
#define DE_485_Pin GPIO_PIN_8
#define DE_485_GPIO_Port GPIOI
#define HV_LED_Pin GPIO_PIN_14
#define HV_LED_GPIO_Port GPIOC
#define LV_LED_Pin GPIO_PIN_10
#define LV_LED_GPIO_Port GPIOI
#define DEBUG_RUN_LED_Pin GPIO_PIN_8
#define DEBUG_RUN_LED_GPIO_Port GPIOG
#define CAN_RUN_LED_Pin GPIO_PIN_7
#define CAN_RUN_LED_GPIO_Port GPIOG
#define TCP_RUN_LED_Pin GPIO_PIN_6
#define TCP_RUN_LED_GPIO_Port GPIOG
/* USER CODE BEGIN Private defines */
extern HAL_StatusTypeDef tcp_server_init;
extern HAL_StatusTypeDef can_init;
extern HAL_StatusTypeDef debug_init;
extern HAL_StatusTypeDef com_init;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
