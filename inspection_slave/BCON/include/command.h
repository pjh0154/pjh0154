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
#define PROMPT "[EP496]"
#define DEBUG_UART_PORT huart1
#define DEBUG_BUFFER_SIZE 64
extern uint8_t debug_put;
#endif

#if defined(COM_UART_MODE_POLLING) || defined(COM_UART_MODE_INTERRUPT) || defined(COM_UART_MODE_DMA)
#define COM_UART_PORT	huart2
//#define COM_BUFFER_SIZE 1024
#define COM_BUFFER_SIZE 2048
extern uint8_t com_buffer_intterrupt[COM_BUFFER_SIZE];
extern int com_buffer_intterrupt_cnt;
extern int com_buffer_cnt;
extern uint8_t com_put;
//void keithley_data_conv(void);
void keithley_data_conv_kgh(char *str);
void BCON_FW_DOWNLOAD(void);
void osg_l_data_read(void);
void osg_r_data_read(void);
void data_new_read(void);
void data_ssd_read(void);
void data_77_read(void);
void data_osg_read(void);
#endif

#endif /* COMMAND_H_ */
