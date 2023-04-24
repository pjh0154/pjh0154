#ifndef timmer_HEADER_H
#define timmer_HEADER_H

#include "global.h"
#include "ep.h"

int createTimer( timer_t *timerID, int sec, int msec );
void timer();
#endif