
#ifndef __CAN_DEV_H__
#define __CAN_DEV_H__

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <pthread.h>

int can_open_port(const char *port);
int can_send_port(struct can_frame *frame);
int can_close_port();

#endif
