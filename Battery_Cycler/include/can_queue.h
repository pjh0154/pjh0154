/*
 * can_queue.h
 *
 *  Created on: 2020. 1. 8.
 *      Author: ghkim
 */

#ifndef INC_CAN_QUEUE_H_
#define INC_CAN_QUEUE_H_

#include "ep.h"

void QUE_INIT(void);
HAL_StatusTypeDef EN_QUE(unsigned int IDE, unsigned int RTR, unsigned int StdId, unsigned int ExtId, char *ptr, unsigned int length);
HAL_StatusTypeDef DE_QUE(void);

#endif /* INC_CAN_QUEUE_H_ */
