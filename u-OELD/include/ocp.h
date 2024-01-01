#ifndef ocp_H
#define ocp_H

#include "global.h"
#include "ep.h"

void *ocp_task();
void ap_ocp_set(int data);
void dp_ocp_set(int data);
void el_ocp_set(int data);

#endif