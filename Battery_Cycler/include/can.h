/*
 * can.h
 *
 *  Created on: 2019. 11. 18.
 *      Author: ghkim
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "ep.h"

//#define CAN_MODE_INTERRUPT
#define CAN_MODE_POLLING

#if defined(CAN_MODE_INTERRUPT) || defined(CAN_MODE_POLLING)
#define CAN_PORT hcan1
#define CAN_USE_FIFO		CAN_FILTER_FIFO0
#define CAN_USE_FILTERMODE	CAN_FILTERMODE_IDMASK // CAN_FILTERMODE_IDLIST
#define FILTERIDHIGH		0x000
#define FILTERIDLOW			0x000
#define FILTERMASKIDHIGH	0x000
#define FILTERMASKIDLOW		0x000

HAL_StatusTypeDef CAN_ConfigFilter(void);
void can_task(void);
HAL_StatusTypeDef can_tx(unsigned int IDE, unsigned int RTR, unsigned int StdId, unsigned int ExtId, char *ptr, unsigned int length);
extern CAN_RxHeaderTypeDef RxMessage;
extern uint8_t can_recv_data[8];
extern uint8_t can_flag;
//extern char temp_str[256];
#endif
#endif /* INC_CAN_H_ */
