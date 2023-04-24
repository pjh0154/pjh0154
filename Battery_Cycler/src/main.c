/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

NOR_HandleTypeDef hnor1;
SRAM_HandleTypeDef hsram2;
NAND_HandleTypeDef hnand1;

osThreadId Task0Handle;
uint32_t Task0Buffer[ 128 ];
osStaticThreadDef_t Task0ControlBlock;
osThreadId Task1Handle;
uint32_t Task1Buffer[ 128 ];
osStaticThreadDef_t Task1ControlBlock;
osThreadId Task2Handle;
uint32_t Task2Buffer[ 128 ];
osStaticThreadDef_t Task2ControlBlock;
osMutexId hMutexHandle;
osStaticMutexDef_t hMutexControlBlock;
/* USER CODE BEGIN PV */
//char watchdog_count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_FSMC_Init(void);
static void MX_IWDG_Init(void);
void Thread0(void const * argument);
void Thread1(void const * argument);
void Thread2(void const * argument);

/* USER CODE BEGIN PFP */
void System_Init(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int thread_operation_flag = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  SCB->VTOR = 0x8004000;
  setvbuf(stdout, NULL, _IONBF, 0); // for printf function
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_CAN1_Init();
  MX_FSMC_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of hMutex */
  osMutexStaticDef(hMutex, &hMutexControlBlock);
  hMutexHandle = osMutexCreate(osMutex(hMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Task0 */
  osThreadStaticDef(Task0, Thread0, osPriorityNormal, 0, 128, Task0Buffer, &Task0ControlBlock);
  Task0Handle = osThreadCreate(osThread(Task0), NULL);

  /* definition and creation of Task1 */
  osThreadStaticDef(Task1, Thread1, osPriorityAboveNormal, 0, 128, Task1Buffer, &Task1ControlBlock);
  Task1Handle = osThreadCreate(osThread(Task1), NULL);

  /* definition and creation of Task2 */
  osThreadStaticDef(Task2, Thread2, osPriorityHigh, 0, 128, Task2Buffer, &Task2ControlBlock);
  Task2Handle = osThreadCreate(osThread(Task2), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 20;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 5;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_8TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 3000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  __HAL_UART_ENABLE_IT(&COM_UART_PORT, UART_IT_RXNE);
  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FAN_ONOFF_GPIO_Port, FAN_ONOFF_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, MC_ONOFF_Pin|DE_485_Pin|LV_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HV_LED_GPIO_Port, HV_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, DEBUG_RUN_LED_Pin|CAN_RUN_LED_Pin|TCP_RUN_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : FAN_ONOFF_Pin */
  GPIO_InitStruct.Pin = FAN_ONOFF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FAN_ONOFF_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EMS_IN_Pin */
  GPIO_InitStruct.Pin = EMS_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(EMS_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MC_ONOFF_Pin DE_485_Pin LV_LED_Pin */
  GPIO_InitStruct.Pin = MC_ONOFF_Pin|DE_485_Pin|LV_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : HV_LED_Pin */
  GPIO_InitStruct.Pin = HV_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HV_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DEBUG_RUN_LED_Pin CAN_RUN_LED_Pin TCP_RUN_LED_Pin */
  GPIO_InitStruct.Pin = DEBUG_RUN_LED_Pin|CAN_RUN_LED_Pin|TCP_RUN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};
  FSMC_NORSRAM_TimingTypeDef ExtTiming = {0};
  FSMC_NAND_PCC_TimingTypeDef ComSpaceTiming = {0};
  FSMC_NAND_PCC_TimingTypeDef AttSpaceTiming = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the NOR1 memory initialization sequence
  */
  hnor1.Instance = FSMC_NORSRAM_DEVICE;
  hnor1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hnor1.Init */
  hnor1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hnor1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hnor1.Init.MemoryType = FSMC_MEMORY_TYPE_NOR;
  hnor1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hnor1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hnor1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hnor1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hnor1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hnor1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hnor1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hnor1.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
  hnor1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_ENABLE;
  hnor1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_B;
  /* ExtTiming */
  ExtTiming.AddressSetupTime = 15;
  ExtTiming.AddressHoldTime = 15;
  ExtTiming.DataSetupTime = 255;
  ExtTiming.BusTurnAroundDuration = 15;
  ExtTiming.CLKDivision = 16;
  ExtTiming.DataLatency = 17;
  ExtTiming.AccessMode = FSMC_ACCESS_MODE_B;

  if (HAL_NOR_Init(&hnor1, &Timing, &ExtTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Perform the SRAM2 memory initialization sequence
  */
  hsram2.Instance = FSMC_NORSRAM_DEVICE;
  hsram2.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram2.Init */
  hsram2.Init.NSBank = FSMC_NORSRAM_BANK3;
  hsram2.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram2.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram2.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram2.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram2.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram2.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram2.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram2.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram2.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram2.Init.ExtendedMode = FSMC_EXTENDED_MODE_ENABLE;
  hsram2.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram2.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */
  ExtTiming.AddressSetupTime = 15;
  ExtTiming.AddressHoldTime = 15;
  ExtTiming.DataSetupTime = 255;
  ExtTiming.BusTurnAroundDuration = 15;
  ExtTiming.CLKDivision = 16;
  ExtTiming.DataLatency = 17;
  ExtTiming.AccessMode = FSMC_ACCESS_MODE_A;

  if (HAL_SRAM_Init(&hsram2, &Timing, &ExtTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Perform the NAND1 memory initialization sequence
  */
  hnand1.Instance = FSMC_NAND_DEVICE;
  /* hnand1.Init */
  hnand1.Init.NandBank = FSMC_NAND_BANK3;
  hnand1.Init.Waitfeature = FSMC_NAND_PCC_WAIT_FEATURE_ENABLE;
  hnand1.Init.MemoryDataWidth = FSMC_NAND_PCC_MEM_BUS_WIDTH_8;
  hnand1.Init.EccComputation = FSMC_NAND_ECC_ENABLE;
  hnand1.Init.ECCPageSize = FSMC_NAND_ECC_PAGE_SIZE_512BYTE;
  hnand1.Init.TCLRSetupTime = 0;
  hnand1.Init.TARSetupTime = 0;
  /* hnand1.Config */
  hnand1.Config.PageSize = 2048;
  hnand1.Config.SpareAreaSize = 64;
  hnand1.Config.BlockSize = 2112;
  hnand1.Config.BlockNbr = 1024;
  hnand1.Config.PlaneNbr = 1;
  hnand1.Config.PlaneSize = 1024;
  hnand1.Config.ExtraCommandEnable = DISABLE;
  /* ComSpaceTiming */
  ComSpaceTiming.SetupTime = 252;
  ComSpaceTiming.WaitSetupTime = 252;
  ComSpaceTiming.HoldSetupTime = 253;
  ComSpaceTiming.HiZSetupTime = 252;
  /* AttSpaceTiming */
  AttSpaceTiming.SetupTime = 252;
  AttSpaceTiming.WaitSetupTime = 252;
  AttSpaceTiming.HoldSetupTime = 252;
  AttSpaceTiming.HiZSetupTime = 252;

  if (HAL_NAND_Init(&hnand1, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */
#if defined(DEBUG_UART_MODE_POLLING) || defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA)
// if you want use printf("%f")
// Add Project->Settings->C/C++ Build->Settings->Tool Settings->C Linker->Miscellaneous->Other options->"-u _printf_float"
int __io_putchar(int ch)
{
	while(HAL_UART_Transmit(&DEBUG_UART_PORT, (uint8_t*)&ch, 1, 1000) != HAL_OK);
	return ch;
}
#endif
void System_Init(void)
{
	printf("F/W Version : %s\r\n",FW_Version);
	printf("============ System Init Start ============\r\n");

	if(hiwdg.Instance->SR == RESET) printf("Watchdog Init OK (%dsec)\r\n",(int)((hiwdg.Init.Reload+1)/(32000/pow(2,hiwdg.Init.Prescaler+2))));
    else printf("Watchdog Init Fail\r\n");

	#ifdef FLASH_USE_ENABLE
	flash_init();
	#endif

	#ifdef DEBUG_UART_MODE_INTERRUPT
	if(HAL_UART_Receive_IT(&DEBUG_UART_PORT, &debug_put, 1) == HAL_OK) printf("DEBUG UART Interrupt Init OK\r\n");
	else printf("DEBUG UART Interrupt Init Fail\r\n");
	#endif

	#ifdef DEBUG_UART_MODE_DMA
	if(HAL_UART_Receive_DMA(&DEBUG_UART_PORT, &debug_put, 1) == HAL_OK) printf("DEBUG UART DMA Init OK\r\n");
	else printf("DEBUG UART DMA Init Fail\r\n");
	#endif

	#ifdef COM_UART_MODE_INTERRUPT
	if(HAL_UART_Receive_IT(&COM_UART_PORT, &com_put, 1) == HAL_OK) printf("COM UART Interrupt Init OK\r\n");
	else printf("COM UART Interrupt Init Fail\r\n");
	#endif

	#ifdef COM_UART_MODE_DMA
	if(HAL_UART_Receive_DMA(&COM_UART_PORT, &com_put, 1) == HAL_OK) printf("COM UART DMA Init OK\r\n");
	else printf("COM UART DMA Init Fail\r\n");
	#endif

	#if defined(CAN_MODE_INTERRUPT) || defined(CAN_MODE_POLLING)
	if(CAN_ConfigFilter() == HAL_OK) printf("CAN Init OK\r\n");
	else printf("CAN Init Fail\r\n");
	#endif

	printf("============= System Init END =============\r\n");
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_Thread0 */
/**
  * @brief  Function implementing the Task0 thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_Thread0 */
void Thread0(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN 5 */
  int i = 0;
  int ii = 0;
  uint32_t phyreg = 0;
  extern struct netif gnetif;

  tcp_echoserver_init();
  /* Infinite loop */
  for(;;)
  {
	if((osSemaphoreWait(lock_tcpip_core,osWaitForever) == osOK) && (osMutexWait(hMutexHandle,osWaitForever) == osOK))
	{
		thread_operation_flag |= (1 << 0);
		if(i++ >= 100)
		{
			i = 0;
			if(HAL_ETH_ReadPHYRegister(&heth, PHY_BSR, &phyreg) == HAL_OK)
			{
				if(((phyreg & PHY_LINKED_STATUS) != 0) && (netif_is_link_up(&gnetif)))
				{
						netif_set_up(&gnetif);
				}
				else
				{
					netif_set_down(&gnetif);
				}
			}
			//HAL_GPIO_TogglePin(TCP_RUN_LED_GPIO_Port, TCP_RUN_LED_Pin);
		}
		if(ii++ >= 10)
		{
			ii = 0;
			HAL_GPIO_TogglePin(TCP_RUN_LED_GPIO_Port, TCP_RUN_LED_Pin);
		}
		tcp_server_tx(NULL);
		while(osMutexRelease(hMutexHandle) != osOK);
		while(osSemaphoreRelease(lock_tcpip_core) != osOK);
	}
	osDelay(50);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_Thread1 */
/**
* @brief Function implementing the Task1 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Thread1 */
void Thread1(void const * argument)
{
  /* USER CODE BEGIN Thread1 */
  int i = 0;
  int ii = 0;
  HAL_GPIO_WritePin(GPIOI, MC_ONOFF_Pin, GPIO_PIN_SET);
  /* Infinite loop */
  for(;;)
  {
	if(osMutexWait(hMutexHandle,osWaitForever) == osOK)
	  //if(1)
	{
		thread_operation_flag |= (1 << 1);
		if(i++ >= 50)
		{
			i = 0;
			HAL_GPIO_TogglePin(DEBUG_RUN_LED_GPIO_Port, DEBUG_RUN_LED_Pin);
		}
		if(ii++ >= 30)
		{
			ii = 0;
			if((psfb.fb_lv_v >= 395) && (psfb.fb_lv_v <= 405))	HAL_GPIO_WritePin(HV_LED_GPIO_Port, HV_LED_Pin, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(LV_LED_GPIO_Port, LV_LED_Pin, GPIO_PIN_RESET);

			if((psfb.fb_hv_v >= 11) && (psfb.fb_lv_v <= 13))	HAL_GPIO_WritePin(LV_LED_GPIO_Port, LV_LED_Pin, GPIO_PIN_SET);
			else HAL_GPIO_WritePin(LV_LED_GPIO_Port, LV_LED_Pin, GPIO_PIN_RESET);

			if(HAL_GPIO_ReadPin(GPIOE, EMS_IN_Pin) == 1)	HAL_GPIO_WritePin(GPIOI, MC_ONOFF_Pin, GPIO_PIN_RESET);
			else	HAL_GPIO_WritePin(GPIOI, MC_ONOFF_Pin, GPIO_PIN_SET);

		}

		#ifdef DEBUG_UART_MODE_POLLING
		if(HAL_UART_Receive(&DEBUG_UART_PORT, &debug_put, 1, 0) == HAL_OK) HAL_UART_RxCpltCallback(&DEBUG_UART_PORT);
		#endif
		#ifdef COM_UART_MODE_POLLING
		if(HAL_UART_Receive(&COM_UART_PORT, &com_put, 1, 0) == HAL_OK) HAL_UART_RxCpltCallback(&COM_UART_PORT);
		#endif

		if(com_buffer_intterrupt_cnt != com_buffer_cnt)
		  {
			  //printf("%x ",com_put);
			  com_put = com_buffer_intterrupt[com_buffer_cnt++];
			  HAL_UART_RxCpltCallback(&COM_UART_PORT);
			  if(com_buffer_cnt >= COM_BUFFER_SIZE) com_buffer_cnt = 0;
		  }


		while(osMutexRelease(hMutexHandle) != osOK);
	}
    osDelay(10);
  }
  /* USER CODE END Thread1 */
}

/* USER CODE BEGIN Header_Thread2 */
/**
* @brief Function implementing the Task2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Thread2 */
void Thread2(void const * argument)
{
  /* USER CODE BEGIN Thread2 */
  System_Init();
  int i = 0;
  /* Infinite loop */
  for(;;)
  {
	if(osMutexWait(hMutexHandle,osWaitForever) == osOK)
	{
	  if(thread_operation_flag == 0x00000003)
	  {
		  thread_operation_flag = 0;
		  HAL_IWDG_Refresh(&hiwdg);
	  }

		if(i++ >= 500)
		{
			i = 0;
			HAL_GPIO_TogglePin(CAN_RUN_LED_GPIO_Port, CAN_RUN_LED_Pin);
		}
		#if defined(CAN_MODE_POLLING)
		if(HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_USE_FIFO))
		{
			  if(HAL_CAN_GetRxMessage(&CAN_PORT, CAN_USE_FIFO, &RxMessage, can_recv_data) == HAL_OK)
			  {
				  can_task();
			  }
		}
		#endif
		DE_QUE();
		while(osMutexRelease(hMutexHandle) != osOK);
	}
	osDelay(1);
  }
  /* USER CODE END Thread2 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
