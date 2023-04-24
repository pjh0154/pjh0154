/*
 * ymodem.c
 *
 *  Created on: 2020. 1. 23.
 *      Author: pjh01
 */
/**
  ******************************************************************************
  * @file    IAP_Main/Src/ymodem.c
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    8-April-2015
  * @brief   This file provides all the software functions related to the ymodem
  *          protocol.
  ******************************************************************************
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/** @addtogroup STM32F1xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "ymodem.h"

#ifdef YMODEM_ENABLE
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  COM_OK       = 0x00,
  COM_ERROR    = 0x01,
  COM_ABORT    = 0x02,
  COM_TIMEOUT  = 0x03,
  COM_DATA     = 0x04,
  COM_LIMIT    = 0x05
} COM_StatusTypeDef;
/*enum
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR,
  FLASHIF_PROTECTION_ERRROR
};*/
/* Private define ------------------------------------------------------------*/
#define YMODEM_RX_UART_PORT huart3
#define YMODEM_TX_UART_PORT huart1
static uint32_t GetSector(uint32_t Address);
NOR_HandleTypeDef hnor1;

#define CRC16_F       /* activate the CRC16 integrity */
#define PACKET_HEADER_SIZE      ((uint32_t)3)
#define PACKET_DATA_INDEX       ((uint32_t)4)
#define PACKET_START_INDEX      ((uint32_t)1)
#define PACKET_NUMBER_INDEX     ((uint32_t)2)
#define PACKET_CNUMBER_INDEX    ((uint32_t)3)
#define PACKET_TRAILER_SIZE     ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE    (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE             ((uint32_t)128)
#define PACKET_1K_SIZE          ((uint32_t)1024)

/* /-------- Packet in IAP memory ------------------------------------------\
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  |
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * \------------------------------------------------------------------------/
 * the first byte is left unused for memory alignment reasons*/

#define FILE_NAME_LENGTH        ((uint32_t)64)
#define FILE_SIZE_LENGTH        ((uint32_t)16)

#define SOH                     ((uint8_t)0x01)  /* start of 128-byte data packet */
#define STX                     ((uint8_t)0x02)  /* start of 1024-byte data packet */
#define ETX                     ((uint8_t)0x03)  /* start of 1024-byte data packet */
#define EOT                     ((uint8_t)0x04)  /* end of transmission */
#define ACK                     ((uint8_t)0x06)  /* acknowledge */
#define NAK                     ((uint8_t)0x15)  /* negative acknowledge */
#define CA                      ((uint32_t)0x18) /* two of these in succession aborts transfer */
#define CRC16                   ((uint8_t)0x43)  /* 'C' == 0x43, request 16-bit CRC */
#define NEGATIVE_BYTE           ((uint8_t)0xFF)

#define ABORT1                  ((uint8_t)0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  ((uint8_t)0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             ((uint32_t)0x100000)
#define DOWNLOAD_TIMEOUT        ((uint32_t)1000) /* One second retry delay */
#define MAX_ERRORS              ((uint32_t)5)

#define TX_TIMEOUT          ((uint32_t)100)

#define IS_CAP_LETTER(c)    (((c) >= 'A') && ((c) <= 'F'))
#define IS_LC_LETTER(c)     (((c) >= 'a') && ((c) <= 'f'))
#define IS_09(c)            (((c) >= '0') && ((c) <= '9'))
#define ISVALIDHEX(c)       (IS_CAP_LETTER(c) || IS_LC_LETTER(c) || IS_09(c))
#define ISVALIDDEC(c)       IS_09(c)
#define CONVERTDEC(c)       (c - '0')

#define CONVERTHEX_ALPHA(c) (IS_CAP_LETTER(c) ? ((c) - 'A'+10) : ((c) - 'a'+10))
#define CONVERTHEX(c)       (IS_09(c) ? ((c) - '0') : CONVERTHEX_ALPHA(c))
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t aFileName[FILE_NAME_LENGTH];
uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
uint8_t TX_RX_FLAG = 0;

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(void);
void FLASH_If_Init(void);
//uint32_t FLASH_If_Erase(uint32_t start);
uint32_t FLASH_If_Erase(uint32_t start, uint32_t size);
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);
void Int2Str(uint8_t *p_str, uint32_t intnum);
uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum);
HAL_StatusTypeDef Serial_PutByte(uint8_t param);
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size);
COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size);
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length);
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk);
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);
extern UART_HandleTypeDef YMODEM_RX_UART_PORT;
extern UART_HandleTypeDef YMODEM_TX_UART_PORT;
HAL_StatusTypeDef nor_flash_wr(uint32_t ptr, uint32_t size);
/* Private functions ---------------------------------------------------------*/

void SerialDownload(void)
{
  uint8_t number[11] = {0};
  uint32_t size = 0;
  COM_StatusTypeDef result;

  TX_RX_FLAG = 0;
  FLASH_If_Init();
  printf("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
  #ifdef DEBUG_UART_MODE_INTERRUPT
  if(HAL_UART_Abort(&YMODEM_RX_UART_PORT) != HAL_OK) printf("Serial Download UART Interrupt Abort Fail\r\n");
  #endif
  __HAL_UART_FLUSH_DRREGISTER(&YMODEM_RX_UART_PORT);
  result = Ymodem_Receive( &size );
  if (result == COM_OK)
  {
	printf("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
	printf("%s",(char *)aFileName);
    Int2Str(number, size);
    printf("\n\r Size: ");
    printf("%s",(char *)number);
    printf(" Bytes\r\n");
    printf("-------------------------------\r\n");
  }
  else if (result == COM_LIMIT)
  {
	  printf("\n\n\rThe image size is higher than the allowed space memory!\n\r");
  }
  else if (result == COM_DATA)
  {
	  printf("\n\n\rVerification failed!\n\r");
  }
  else if (result == COM_ABORT)
  {
	  printf("\r\n\nAborted by user.\n\r");
  }
  else
  {
	  printf("\n\rFailed to receive the file!\n\r");
  }
}

/*void SerialUpload(void)
{
  uint8_t flag = 0;
  uint8_t status = 0;
  uint8_t ymodem_tx = 0;
  uint8_t reset_req[5] = {STX,'R','S','T',ETX};
  TX_RX_FLAG = 1;
  FLASH_If_Init();

  while(flag < 3)
  {
	  switch (flag)
	  {
	  case 0 :
		  if(HAL_UART_Abort(&YMODEM_TX_UART_PORT) != HAL_OK) printf("TP1\r\n");
		  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
		  if(HAL_UART_Transmit(&YMODEM_TX_UART_PORT, reset_req, sizeof(reset_req), TX_TIMEOUT)) printf("TP2\r\n");
		  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);

		  if(HAL_UART_Receive(&YMODEM_TX_UART_PORT, &status, 1, DOWNLOAD_TIMEOUT*5) == HAL_OK)
		  {
			  if(status == '.')
			  {
				  ymodem_tx = 'D';
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
				  HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &ymodem_tx, 1, TX_TIMEOUT);
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
				  flag = 1;
			  }
		  }
		  else printf("TP3\r\n");
		  break;
	  case 1 :
		  if(HAL_UART_Receive(&YMODEM_TX_UART_PORT, &status, 1, DOWNLOAD_TIMEOUT) == HAL_OK)
		  {
			  if(status == 'T')
			  {
				  ymodem_tx = '1';
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
				  HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &ymodem_tx, 1, TX_TIMEOUT);
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
				  flag = 2;
			  }
		  }
		  break;
	  case 2 :
		  if(HAL_UART_Receive(&YMODEM_TX_UART_PORT, &status, 1, DOWNLOAD_TIMEOUT) == HAL_OK)
		  {
			  if(status == CRC16)
			  {
				  status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, (const uint8_t*)"PCTEST.binary", USER_FLASH_SIZE);
				  if (status != COM_OK)
				  {
					printf("\n\rError Occurred while Transmitting File\n\r");
					flag = 3;
				  }
				  else
				  {
					printf("\n\rFile uploaded successfully \n\r");
					flag = 3;
				  }
			  }
		  }
		  break;
	  default :
		  break;
	  }
  }
}*/

void SerialUpload(void)
{
  uint8_t flag = 0;
  uint8_t status = 0;
  uint8_t ymodem_tx = 0;
  static uint8_t reset_req[5] = {STX,'R','S','T',ETX};

  TX_RX_FLAG = 1;
  FLASH_If_Init();

  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET);
  HAL_UART_Transmit(&YMODEM_TX_UART_PORT, reset_req, sizeof(reset_req), TX_TIMEOUT);
  //HAL_UART_Transmit_IT(&YMODEM_TX_UART_PORT, reset_req, 5);
  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET);
  while(flag < 3)
 //if(flag<3)
  {
	  switch (flag)
	  {
	  case 0 :
		  HAL_UART_Abort(&YMODEM_TX_UART_PORT);
/*		  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
		  if(HAL_UART_Transmit(&YMODEM_TX_UART_PORT, reset_req, sizeof(reset_req), TX_TIMEOUT)) printf("TP2\r\n");
		  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
		  printf("TP01\r\n");*/
		  if(HAL_UART_Receive(&YMODEM_TX_UART_PORT, &status, 1, DOWNLOAD_TIMEOUT*5) == HAL_OK)
		  {
			  if(status == '.')
			  {
				  ymodem_tx = 'D';
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
				  HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &ymodem_tx, 1, TX_TIMEOUT);
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
				  flag = 1;
			  }
		  }
		  break;
	  case 1 :
		  if(HAL_UART_Receive(&YMODEM_TX_UART_PORT, &status, 1, DOWNLOAD_TIMEOUT) == HAL_OK)
		  {
			  if(status == 'T')
			  {
				  ymodem_tx = '1';
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
				  HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &ymodem_tx, 1, TX_TIMEOUT);
				  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
				  flag = 2;
			  }
		  }
		  break;
	  case 2 :
		  if(HAL_UART_Receive(&YMODEM_TX_UART_PORT, &status, 1, DOWNLOAD_TIMEOUT) == HAL_OK)
		  {
			  if(status == CRC16)
			  {
				  status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, (const uint8_t*)"PCTEST.binary", USER_FLASH_SIZE);
				  if (status != COM_OK)
				  {
					printf("\n\rError Occurred while Transmitting File\n\r");
					flag = 3;
				  }
				  else
				  {
					printf("\n\rFile uploaded successfully \n\r");
					flag = 3;
				  }
			  }
				flag = 3;
		  }
		  break;
	  default :
		  break;
	  }
  }
}



/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static HAL_StatusTypeDef ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
  uint32_t crc;
  uint32_t packet_size = 0;
  HAL_StatusTypeDef status;
  uint8_t char1;

  *p_length = 0;
  status = HAL_UART_Receive(&YMODEM_RX_UART_PORT, &char1, 1, timeout);
  if (status == HAL_OK)
  {
    switch (char1)
    {
      case SOH:
        packet_size = PACKET_SIZE;
        break;
      case STX:
        packet_size = PACKET_1K_SIZE;
        break;
      case EOT:
        break;
      case CA:
        if ((HAL_UART_Receive(&YMODEM_RX_UART_PORT, &char1, 1, timeout) == HAL_OK) && (char1 == CA))
        {
          packet_size = 2;
        }
        else
        {
          status = HAL_ERROR;
        }
        break;
      case ABORT1:
      case ABORT2:
        status = HAL_BUSY;
        break;
      default:
        status = HAL_ERROR;
        break;
    }
    *p_data = char1;

    if (packet_size >= PACKET_SIZE )
    {
      status = HAL_UART_Receive(&YMODEM_RX_UART_PORT, &p_data[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, timeout);

      /* Simple packet sanity check */
      if (status == HAL_OK )
      {
        if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
        {
          packet_size = 0;
          status = HAL_ERROR;
        }
        else
        {
          /* Check packet CRC */
          crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
          crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
          if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc )
          {
            packet_size = 0;
            status = HAL_ERROR;
          }
        }
      }
      else
      {
        packet_size = 0;
      }
    }
  }
  *p_length = packet_size;
  return status;
}

/**
  * @brief  Prepare the first block
  * @param  p_data:  output buffer
  * @param  p_file_name: name of the file to be sent
  * @param  length: length of the file to be sent in bytes
  * @retval None
  */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
  uint32_t i, j = 0;
  uint8_t astring[10] = {0};

  /* first 3 bytes are constant */

  p_data[PACKET_START_INDEX] = SOH;
  p_data[PACKET_NUMBER_INDEX] = 0x00;
  p_data[PACKET_CNUMBER_INDEX] = 0xff;


  /* Filename written */
  for (i = 0; (p_file_name[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
  {
    p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
  }

  p_data[i + PACKET_DATA_INDEX] = 0x00;

  /* file size written */
  Int2Str (astring, length);
  i = i + PACKET_DATA_INDEX + 1;
  while (astring[j] != '\0')
  {
    p_data[i++] = astring[j++];
  }

  /* padding with zeros */
  for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++)
  {
    p_data[j] = 0;
  }
}

/**
  * @brief  Prepare the data packet
  * @param  p_source: pointer to the data to be sent
  * @param  p_packet: pointer to the output buffer
  * @param  pkt_nr: number of the packet
  * @param  size_blk: length of the block to be sent in bytes
  * @retval None
  */
static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk)
{
  uint8_t *p_record;
  uint32_t i, size, packet_size;

  /* Make first three packet */
  packet_size = size_blk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
  size = size_blk < packet_size ? size_blk : packet_size;
  if (packet_size == PACKET_1K_SIZE)
  {
    p_packet[PACKET_START_INDEX] = STX;
  }
  else
  {
    p_packet[PACKET_START_INDEX] = SOH;
  }
  p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
  p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
  p_record = p_source;

  /* Filename packet has valid data */
  for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX;i++)
  {
    p_packet[i] = *p_record++;
  }
  if ( size  <= packet_size)
  {
    for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++)
    {
      p_packet[i] = 0x1A;  /* EOF (0x1A) or 0x00 */
    }
  }
}

/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
  uint32_t crc = crc_in;
  uint32_t in = byte | 0x100;

  do
  {
    crc <<= 1;
    in <<= 1;
    if(in & 0x100)
      ++crc;
    if(crc & 0x10000)
      crc ^= 0x1021;
  }

  while(!(in & 0x10000));

  return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
  uint32_t crc = 0;
  const uint8_t* dataEnd = p_data+size;

  while(p_data < dataEnd)
    crc = UpdateCRC16(crc, *p_data++);

  crc = UpdateCRC16(crc, 0);
  crc = UpdateCRC16(crc, 0);

  return crc&0xffffu;
}

/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
  uint32_t sum = 0;
  const uint8_t *p_data_end = p_data + size;

  while (p_data < p_data_end )
  {
    sum += *p_data++;
  }

  return (sum & 0xffu);
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @retval COM_StatusTypeDef result of reception/programming
  */
COM_StatusTypeDef Ymodem_Receive ( uint32_t *p_size )
{
  uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
  uint32_t ramsource, filesize;
  //uint32_t flashdestination;
  uint8_t *file_ptr;
  uint8_t file_size[FILE_SIZE_LENGTH], tmp, packets_received;
  COM_StatusTypeDef result = COM_OK;

  /* Initialize flashdestination variable */
  //flashdestination = APPLICATION_ADDRESS;
  //flashdestination = NOR_MEMORY_ADRESS1;
  printf("PTPT1\r\n");
  while ((session_done == 0) && (result == COM_OK))
  {
    packets_received = 0;
    file_done = 0;
    printf("PTPT2\r\n");
    while ((file_done == 0) && (result == COM_OK))
    {
      switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT))
      {
        case HAL_OK:
          errors = 0;
          switch (packet_length)
          {
            case 2:
              /* Abort by sender */
              Serial_PutByte(ACK);
              result = COM_ABORT;
              break;
            case 0:
              /* End of transmission */
              Serial_PutByte(ACK);
              file_done = 1;
              break;
            default:
              /* Normal packet */
              if (aPacketData[PACKET_NUMBER_INDEX] != packets_received)
              {
                Serial_PutByte(NAK);
              }
              else
              {
                if (packets_received == 0)
                {
                  /* File name packet */
                  if (aPacketData[PACKET_DATA_INDEX] != 0)
                  {
                    /* File name extraction */
                    i = 0;
                    file_ptr = aPacketData + PACKET_DATA_INDEX;
                    while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH))
                    {
                      aFileName[i++] = *file_ptr++;
                    }

                    /* File size extraction */
                    aFileName[i++] = '\0';
                    i = 0;
                    file_ptr ++;
                    while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
                    {
                      file_size[i++] = *file_ptr++;
                    }
                    file_size[i++] = '\0';
                    Str2Int(file_size, &filesize);

                    /* Test the size of the image to be sent */
                    /* Image size is greater than Flash size */
                    if (*p_size > (USER_FLASH_SIZE + 1))
                    {
                      /* End session */
                      tmp = CA;
                      HAL_UART_Transmit(&YMODEM_RX_UART_PORT, &tmp, 1, NAK_TIMEOUT);
                      HAL_UART_Transmit(&YMODEM_RX_UART_PORT, &tmp, 1, NAK_TIMEOUT);
                      result = COM_LIMIT;
                    }
                    /* erase user application area */
                    //FLASH_If_Erase(APPLICATION_ADDRESS,filesize);
                    HAL_NOR_Erase_Block(&hnor1, filesize, NOR_MEMORY_ADRESS1);
                    *p_size = filesize;
                    Serial_PutByte(ACK);
                    Serial_PutByte(CRC16);
                  }
                  /* File header packet is empty, end session */
                  else
                  {
                    Serial_PutByte(ACK);
                    file_done = 1;
                    session_done = 1;
                    break;
                  }
                  printf("PT2\r\n");
                }

                else /* Data packet */
                {
                    printf("PT3\r\n");
                	ramsource = (uint32_t) &aPacketData[PACKET_DATA_INDEX];

                  if(nor_flash_wr((uint32_t)ramsource, packet_length)== HAL_OK)
                  {
                      printf("PT4\r\n");
                	  Serial_PutByte(ACK);
                  }
                  else
                  {
                      /* End session */
                      Serial_PutByte(CA);
                      Serial_PutByte(CA);
                      result = COM_DATA;
                  }

                  /* Write received data in Flash */
/*                  if (FLASH_If_Write(flashdestination, (uint32_t*) ramsource, packet_length/4) == FLASHIF_OK)
                  {
                    flashdestination += packet_length;
                    Serial_PutByte(ACK);
                  }*/

                  /* An error occurred while writing to Flash memory */
                //  else
                //  {
                //    /* End session */
                //    Serial_PutByte(CA);
                //    Serial_PutByte(CA);
                //    result = COM_DATA;
                //  }
                }
                packets_received++;
                session_begin = 1;
              }
              break;
          }
          break;
        case HAL_BUSY: /* Abort actually */
          Serial_PutByte(CA);
          Serial_PutByte(CA);
          result = COM_ABORT;
          break;
        default:
          if (session_begin > 0)
          {
            errors ++;
          }
          if (errors > MAX_ERRORS)
          {
            /* Abort communication */
            Serial_PutByte(CA);
            Serial_PutByte(CA);
          }
          else
          {
            Serial_PutByte(CRC16); /* Ask for a packet */
          }
          break;
      }
    }
  }
  return result;
}

/**
  * @brief  Transmit a file using the ymodem protocol
  * @param  p_buf: Address of the first byte
  * @param  p_file_name: Name of the file sent
  * @param  file_size: Size of the transmission
  * @retval COM_StatusTypeDef result of the communication
  */
COM_StatusTypeDef Ymodem_Transmit (uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size)
{
  uint32_t errors = 0, ack_recpt = 0, size = 0, pkt_size;
  uint8_t *p_buf_int;
  COM_StatusTypeDef result = COM_OK;
  uint32_t blk_number = 1;
  uint8_t a_rx_ctrl[2];
  uint8_t i;
#ifdef CRC16_F
  uint32_t temp_crc;
#else /* CRC16_F */
  uint8_t temp_chksum;
#endif /* CRC16_F */
  /* Prepare first block - header */
  PrepareIntialPacket(aPacketData, p_file_name, file_size);
  while (( !ack_recpt ) && ( result == COM_OK ))
  {
	  /* Send Packet */
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET);
	  while(HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT) != HAL_OK);
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET);
    /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */

    /* Wait for Ack and 'C' */
    if (HAL_UART_Receive(&YMODEM_TX_UART_PORT, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
    	if (a_rx_ctrl[0] == ACK)
      {
    		ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
        if ((HAL_UART_Receive(&YMODEM_TX_UART_PORT, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
        {
        	HAL_Delay( 2 );
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }
    if (errors >= MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  p_buf_int = p_buf;
  size = file_size;
  /* Here 1024 bytes length is used to send the packets */
  while ((size) && (result == COM_OK ))
  {
	  /* Prepare next packet */
    PreparePacket(p_buf_int, aPacketData, blk_number, size);
    ack_recpt = 0;
    a_rx_ctrl[0] = 0;
    errors = 0;

    /* Resend packet if NAK for few times else end of communication */
    while (( !ack_recpt ) && ( result == COM_OK ))
    {
      /* Send next packet */
      if (size >= PACKET_1K_SIZE)
      {
        pkt_size = PACKET_1K_SIZE;
      }
      else
      {
        pkt_size = PACKET_SIZE;
      }

	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET);
      HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &aPacketData[PACKET_START_INDEX], pkt_size + PACKET_HEADER_SIZE, NAK_TIMEOUT);
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET);
      /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F
      temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_crc >> 8);
      Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */
      temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], pkt_size);
      Serial_PutByte(temp_chksum);
#endif /* CRC16_F */
      __HAL_UART_FLUSH_DRREGISTER(&YMODEM_TX_UART_PORT);
      /* Wait for Ack */
      if ((HAL_UART_Receive(&YMODEM_TX_UART_PORT, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == ACK))
      {
        ack_recpt = 1;
        if (size > pkt_size)
        {
          p_buf_int += pkt_size;
          size -= pkt_size;
          if (blk_number == (USER_FLASH_SIZE / PACKET_1K_SIZE))
          {
            result = COM_LIMIT; /* boundary error */
          }
          else
          {
            blk_number++;
          }
        }
        else
        {
          p_buf_int += pkt_size;
          size = 0;
        }
      }
      else
      {
        errors++;
      }

      /* Resend packet if NAK  for a count of 10 else end of communication */
      if (errors >= MAX_ERRORS)
      {
        result = COM_ERROR;
      }
    }
  }

  /* Sending End Of Transmission char */
  ack_recpt = 0;
  a_rx_ctrl[0] = 0x00;
  errors = 0;
  while (( !ack_recpt ) && ( result == COM_OK ))
  {
    Serial_PutByte(EOT);
    __HAL_UART_FLUSH_DRREGISTER(&YMODEM_TX_UART_PORT);
    /* Wait for Ack */
    if (HAL_UART_Receive(&YMODEM_TX_UART_PORT, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == ACK)
      {
        ack_recpt = 1;
      }
      else if (a_rx_ctrl[0] == CA)
      {
    	__HAL_UART_FLUSH_DRREGISTER(&YMODEM_TX_UART_PORT);
        if ((HAL_UART_Receive(&YMODEM_TX_UART_PORT, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK) && (a_rx_ctrl[0] == CA))
        {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&YMODEM_TX_UART_PORT);
          result = COM_ABORT;
        }
      }
    }
    else
    {
      errors++;
    }

    if (errors >=  MAX_ERRORS)
    {
      result = COM_ERROR;
    }
  }

  /* Empty packet sent - some terminal emulators need this to close session */
  if ( result == COM_OK )
  {
    /* Preparing an empty packet */
    aPacketData[PACKET_START_INDEX] = SOH;
    aPacketData[PACKET_NUMBER_INDEX] = 0;
    aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
    for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
    {
      aPacketData [i] = 0x00;
    }

    /* Send Packet */
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET);
    HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE, NAK_TIMEOUT);
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET);

    /* Send CRC or Check Sum based on CRC16_F */
#ifdef CRC16_F
    temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_crc >> 8);
    Serial_PutByte(temp_crc & 0xFF);
#else /* CRC16_F */
    temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    Serial_PutByte(temp_chksum);
#endif /* CRC16_F */
    __HAL_UART_FLUSH_DRREGISTER(&YMODEM_TX_UART_PORT);
    /* Wait for Ack and 'C' */
    if (HAL_UART_Receive(&YMODEM_TX_UART_PORT, &a_rx_ctrl[0], 1, NAK_TIMEOUT) == HAL_OK)
    {
      if (a_rx_ctrl[0] == CA)
      {
          HAL_Delay( 2 );
          __HAL_UART_FLUSH_DRREGISTER(&YMODEM_TX_UART_PORT);
          result = COM_ABORT;
      }
    }
  }

  return result; /* file transmitted successfully */
}

/**
  * @brief  Convert an Integer to a string
  * @param  p_str: The string output pointer
  * @param  intnum: The integer to be converted
  * @retval None
  */
void Int2Str(uint8_t *p_str, uint32_t intnum)
{
  uint32_t i, divider = 1000000000, pos = 0, status = 0;

  for (i = 0; i < 10; i++)
  {
    p_str[pos++] = (intnum / divider) + 48;

    intnum = intnum % divider;
    divider /= 10;
    if ((p_str[pos-1] == '0') & (status == 0))
    {
      pos = 0;
    }
    else
    {
      status++;
    }
  }
}

/**
  * @brief  Convert a string to an integer
  * @param  p_inputstr: The string to be converted
  * @param  p_intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *p_inputstr, uint32_t *p_intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if ((p_inputstr[0] == '0') && ((p_inputstr[1] == 'x') || (p_inputstr[1] == 'X')))
  {
    i = 2;
    while ( ( i < 11 ) && ( p_inputstr[i] != '\0' ) )
    {
      if (ISVALIDHEX(p_inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(p_inputstr[i]);
      }
      else
      {
        /* Return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }

    /* valid result */
    if (p_inputstr[i] == '\0')
    {
      *p_intnum = val;
      res = 1;
    }
  }
  else /* max 10-digit decimal input */
  {
    while ( ( i < 11 ) && ( res != 1 ) )
    {
      if (p_inputstr[i] == '\0')
      {
        *p_intnum = val;
        /* return 1 */
        res = 1;
      }
      else if (((p_inputstr[i] == 'k') || (p_inputstr[i] == 'K')) && (i > 0))
      {
        val = val << 10;
        *p_intnum = val;
        res = 1;
      }
      else if (((p_inputstr[i] == 'm') || (p_inputstr[i] == 'M')) && (i > 0))
      {
        val = val << 20;
        *p_intnum = val;
        res = 1;
      }
      else if (ISVALIDDEC(p_inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(p_inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
      i++;
    }
  }

  return res;
}

/**
  * @brief  Transmit a byte to the HyperTerminal
  * @param  param The byte to be sent
  * @retval HAL_StatusTypeDef HAL_OK if OK
  */

HAL_StatusTypeDef Serial_PutByte( uint8_t param )
{
	HAL_StatusTypeDef result = HAL_ERROR;
   /*May be timeouted...*/
  if(TX_RX_FLAG == 0)
  {
	  if ( YMODEM_RX_UART_PORT.gState == HAL_UART_STATE_TIMEOUT )
	  {
		  YMODEM_RX_UART_PORT.gState = HAL_UART_STATE_READY;
	  }

	 // return HAL_UART_Transmit(&YMODEM_RX_UART_PORT, &param, 1, TX_TIMEOUT);
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET);
	  //return HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &param, 1, TX_TIMEOUT);
	  result = HAL_UART_Transmit(&YMODEM_RX_UART_PORT, &param, 1, TX_TIMEOUT);
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET);
	  return result;
  }
  else
  {
	  if ( YMODEM_TX_UART_PORT.gState == HAL_UART_STATE_TIMEOUT )
	  {
		  YMODEM_TX_UART_PORT.gState = HAL_UART_STATE_READY;
	  }
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_SET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_SET);
	  //return HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &param, 1, TX_TIMEOUT);
	  result = HAL_UART_Transmit(&YMODEM_TX_UART_PORT, &param, 1, TX_TIMEOUT);
	  HAL_GPIO_WritePin(DE_485_GPIO_Port, DE_485_Pin, GPIO_PIN_RESET);
	  while(HAL_GPIO_ReadPin(DE_485_GPIO_Port, DE_485_Pin) != GPIO_PIN_RESET);
	  return result;
  }
}

void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

uint32_t FLASH_If_Erase(uint32_t start, uint32_t size)
{
	  uint32_t PageError = 0;
	  FLASH_EraseInitTypeDef pEraseInit;
	  HAL_StatusTypeDef status = HAL_OK;

	  /* Unlock the Flash to enable the flash control register access *************/
	  HAL_FLASH_Unlock();

	  pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	  pEraseInit.Banks = FLASH_BANK_1;
	  pEraseInit.Sector = GetSector(start);
	  pEraseInit.NbSectors = (GetSector(start + size) - GetSector(start)) + 1;
	  pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	  status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

	  /* Lock the Flash to disable the flash control register access (recommended
	     to protect the FLASH memory against possible unwanted operation) *********/
	  HAL_FLASH_Lock();

	  if (status != HAL_OK)
	  {
	    /* Error occurred while page erase */
	    return FLASHIF_ERASEKO;
	  }

	  return FLASHIF_OK;

}

uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
  uint32_t i = 0;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  for (i = 0; (i < length) && (destination <= (USER_FLASH_END_ADDRESS-4)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(p_source+i)) == HAL_OK)
    {
     /* Check the written value */
      if (*(uint32_t*)destination != *(uint32_t*)(p_source+i))
      {
        /* Flash content doesn't match SRAM content */
        return(FLASHIF_WRITINGCTRL_ERROR);
      }
      /* Increment FLASH destination address */
      destination += 4;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (FLASHIF_WRITING_ERROR);
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return (FLASHIF_OK);
}

static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_SECTOR_11;
  }
    return sector;
}



#endif
/*******************(C)COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
