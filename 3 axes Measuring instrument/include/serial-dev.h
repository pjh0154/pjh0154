#ifndef __SERIAL_DEV_H__
#define __SERIAL_DEV_H__

int sdcd_serial_open(int baud, int dataBit, int stopBit, int parity);
int sdcd_serial_close();
int sdcd_serial_write(int len, char *cmdBuf);
unsigned long func_timecheck_start();
unsigned long func_timecheck_end();



#endif
