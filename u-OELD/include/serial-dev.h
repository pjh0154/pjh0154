#ifndef __SERIAL_DEV_H__
#define __SERIAL_DEV_H__

int sdcd_serial_open(int baud, int dataBit, int stopBit, int parity);
int sdcd_serial_close();
int sdcd_serial_write(int len, char *cmdBuf);
unsigned long func_timecheck_start();
unsigned long func_timecheck_end();
void com_task(void);
void data_check(void);
void com_read_data_init(void);
void uart_ack(void);
void uart_nack(void);
void com_task_ack(unsigned char cmd);
void bist_ack(unsigned char mode, unsigned char result);
void com_sensing_task_ack(unsigned char cmd, unsigned char *avdd_cur, unsigned char *elvss_cur);
void uart_index_ack(unsigned char index);
void pg_reboot_ack(void);
void ext_com_task_ack(unsigned char cmd, unsigned char len, unsigned char *data);

#endif
