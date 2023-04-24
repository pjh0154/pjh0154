/*
 * can_queue.c
 *
 *  Created on: 2020. 1. 8.
 *      Author: ghkim
 */

#include "can_queue.h"

int IsEmpty(void);
int IsFull(void);

#define QUE_MAX 		256
#define MAX_FRAME_SIZE	8
int front;
int rear;

typedef struct{
	unsigned int IDE;
	unsigned int RTR;
	unsigned int StdId;
	unsigned int ExtId;
	char str[MAX_FRAME_SIZE];
	unsigned int length;
}QUE_BUFFER;
QUE_BUFFER qBuf[QUE_MAX];

void QUE_INIT(void)
{
	front = -1;
	rear = -1;
	memset(qBuf, 0, sizeof(qBuf));
}

int IsEmpty(void)
{
	if(front == rear) return 0;
	else return 1;
}

int IsFull(void)
{
	int tmp = (rear + 1) % QUE_MAX;
	if(tmp == front) return 0;
	else return 1;
}

HAL_StatusTypeDef EN_QUE(unsigned int IDE, unsigned int RTR, unsigned int StdId, unsigned int ExtId, char *ptr, unsigned int length)
{
	if(IsFull())
	{
		rear = (rear + 1) % QUE_MAX;
		qBuf[rear].IDE = IDE;
		qBuf[rear].RTR = RTR;
		qBuf[rear].StdId = StdId;
		qBuf[rear].ExtId = ExtId;
		qBuf[rear].length = length;
		memset(&qBuf[rear].str[0], 0, MAX_FRAME_SIZE);
		memcpy(&qBuf[rear].str[0], ptr, length);
		return HAL_OK;
	}
	else return HAL_ERROR;
}

HAL_StatusTypeDef DE_QUE(void)
{
	int tmp = (front + 1) % QUE_MAX;

	if(IsEmpty())
	{
		if(can_tx(qBuf[tmp].IDE, qBuf[tmp].RTR, qBuf[tmp].StdId, qBuf[tmp].ExtId, &qBuf[tmp].str[0], qBuf[tmp].length) == HAL_OK)
		{
			front = (front + 1) % QUE_MAX;
			return HAL_OK;
		}
		else return HAL_ERROR;
	}
	else return HAL_OK;
}
