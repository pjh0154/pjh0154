#ifndef __SERIAL_DEV_2_H__
#define __SERIAL_DEV_2_H__

//void *ex_port_serial_recv_thread(void *data); 
void ex_port_serial_task(void); 
int ex_port_serial_write(int len, char *cmdBuf);
int ex_port_serial_open(int baud, int dataBit, int stopBit, int parity);
void ex_port_ack(void);
static void ex_port_com_task(char *ptr);
void ex_port_data_check(void);
void ex_port_send_task(unsigned char *cmd, unsigned int len);
void ex_com_read_data_init(void);
#endif
