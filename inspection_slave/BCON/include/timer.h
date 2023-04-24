/*
 * timer.h
 *
 *  Created on: 2019. 4. 25.
 *      Author: ghkim
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "ep.h"

#define TIMER1_MODE_INTERRUPT

#ifdef TIMER1_MODE_INTERRUPT
#define TIMER1_CHANNEL htim1
#define TIMER1_REF_CLK 48000000
extern void timer1_task(void);
#endif

#endif /* TIMER_H_ */
