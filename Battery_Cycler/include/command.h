/*
 * command.h
 *
 *  Created on: 2019. 4. 25.
 *      Author: ghkim
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "ep.h"

//#define DEBUG_UART_MODE_INTERRUPT
#define DEBUG_UART_MODE_POLLING
//#define DEBUG_UART_MODE_DMA

#define COM_UART_MODE_INTERRUPT
//#define COM_UART_MODE_POLLING
//#define COM_UART_MODE_DMA

#if defined(DEBUG_UART_MODE_POLLING) || defined(DEBUG_UART_MODE_INTERRUPT) || defined(DEBUG_UART_MODE_DMA)
void debug(char *ptr);
#define PROMPT "[MCBSTM32F200]"
#define DEBUG_UART_PORT	huart3
#define DEBUG_BUFFER_SIZE 128
#define acdc_on 0xF0
#define acdc_off 0x0F
#define psfb_on 1
#define psfb_off 0
#define charge 0x01
#define discharge 0x02
extern uint8_t debug_put;
extern uint8_t cprf;
extern uint8_t manual_flag;
extern uint8_t flag1;
extern unsigned char charge_discharge[100];
#endif

#if defined(COM_UART_MODE_POLLING) || defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA)
#define COM_UART_PORT	huart1
#define COM_BUFFER_SIZE 256
extern HAL_StatusTypeDef com_tx(char *str);
extern uint8_t com_put;
extern int com_buffer_intterrupt_cnt;
extern int com_buffer_cnt;
extern uint8_t com_buffer_intterrupt[COM_BUFFER_SIZE];
extern uint8_t psfb_flag;
extern uint8_t acdc_flag;
extern uint8_t dis_acdc_flag;
extern uint8_t acdc_ovp;
extern uint8_t psfb_ovp;
HAL_StatusTypeDef flash_read(void);
HAL_StatusTypeDef flash_write(void);
void flash_id(void);
void nor_flash_id(void);
void charge_mode(void);
void charge_acdc(void);
void charge_psfb(void);
void auto_off(void);
void auto_off1(void);
void auto_off2(void);
void auto_off3(void);
HAL_StatusTypeDef nor_flash_write(void);
HAL_StatusTypeDef nor_flash_read(void);
void nor_flash_erase(void);
HAL_StatusTypeDef sram_write(void);
HAL_StatusTypeDef sram_read(void);
HAL_StatusTypeDef sram_write1(void);
//void SerialDownload(void);

#endif

#endif /* COMMAND_H_ */
