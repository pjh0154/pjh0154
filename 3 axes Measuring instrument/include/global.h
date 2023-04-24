//
// Created by hong on 2022-04-12.
//

#ifndef _GLOBAL_H
#define _GLOBAL_H
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <setjmp.h>
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <termios.h>
#include <omp.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <linux/rtnetlink.h>
#include <linux/wireless.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdbool.h>
#include <linux/socket.h>
#include <netinet/tcp.h>

#define FW_VERSION "1.0.1.9"
#define ep961_ver 0x0018

#define __MAX_BUF_SIZE 1024
#define MAX_STR_LEN 128
#define MAX_PATH 260
#define MAX_IP_ADDR 20

#define LOGD(...) printf(__VA_ARGS__)
#define LOGE(...) printf(__VA_ARGS__)

#endif
