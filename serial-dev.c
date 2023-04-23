#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <pthread.h>
#include <termios.h>                  // B115200, CS8 �� ��� ����
#include <fcntl.h>                    // O_RDWR , O_NOCTTY ���� ��� ����

#include "../include/serial-dev.h"
#include "../include/global.h"

#define PORT_DEVICE	"/dev/ttyPS1"


#define READ_SIZE	256
#define BUFF_SIZE	256
#define CMD_TIMEOUT		2000	//1000 msec
#define SET_TIMEOUT		100

int thr_serial_id;
int fd = -1;
char recvBuf[BUFF_SIZE];
int recvCnt = 0;
//int waitTime = 100; //default 100ms

struct pollfd	poll_events;      //üũ�� event ������ ���� struct
pthread_t recvThread = 0;
int thread_serial_end = 0;

// converts integer baud to Linux define
static int get_baud(int baud)
{
	switch (baud) {
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 230400:
			return B230400;
		case 460800:
			return B460800;
		case 500000:
			return B500000;
		case 576000:
			return B576000;
		case 921600:
			return B921600;
		case 1000000:
			return B1000000;
		case 1152000:
			return B1152000;
		case 1500000:
			return B1500000;
		case 2000000:
			return B2000000;
		case 2500000:
			return B2500000;
		case 3000000:
			return B3000000;
		case 3500000:
			return B3500000;
		case 4000000:
			return B4000000;
		default:
			return -1;
	}
}

//static int sdcd_serial_write(int len, char *cmdBuf) {
int sdcd_serial_write(int len, char *cmdBuf) {	
	int res = 0;

	// fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_write() len:%d, cmd:%s\n", len, cmdBuf);
	res = write(fd, cmdBuf, len);
	if (res < 0) {
		fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_write() fail - len:%d, cmd:%s\n", len, cmdBuf);
		return res;
	}

	return res;
}
struct timespec 	func_time;
unsigned long func_timecheck_start()
{
	clock_gettime(CLOCK_MONOTONIC, &func_time);

	return 0;
}

unsigned long func_timecheck_end()
{
	struct timespec 	end_time;
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	return (((end_time.tv_sec - func_time.tv_sec)*1000000000) + (end_time.tv_nsec - func_time.tv_nsec))/1000000;//ms
}


void *serial_recv_thread(void *data) 
{
	int poll_state;
	int cnt = 0;
	int i=0;
	int packet_end=0;
	char readBuf[READ_SIZE];
	int j;

	recvCnt = 0;
	memset (readBuf, 0, READ_SIZE);
	memset (recvBuf, 0, BUFF_SIZE);

	while (!thread_serial_end)
	{
		poll_state = poll(                                 // poll()�� ȣ���Ͽ� event �߻� ���� Ȯ��
			(struct pollfd*)&poll_events,  // event ��� �����
			1,  // üũ�� pollfd ����
			1000	// time out �ð�
		);
		// fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_recv() in 2\n");
		if (poll_state > 0)                             // �߻��� event �� ����
		{
			if (poll_events.revents & POLLIN)            // event �� �ڷ� ����?
			{
				cnt = read(fd, readBuf, READ_SIZE);

				if (cnt > 0) {
					for(j=0; j<cnt; j++) printf("%c    %x\r\n", readBuf[j], readBuf[j]);
					//LOGD("[%s]%s\n",__func__ ,readBuf);

				}
				if (poll_events.revents & POLLERR)      // event �� ����?
				{
					
					fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_recv() poll_events error!\n");
				//break;
				}
			}
			else if (poll_state < 0) {
				fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_recv() Critial Error!\n");
				packet_end=1;
				break;
			}
			if(thread_serial_end>0) break;
		}
	}
	return NULL;
}


int sdcd_serial_open(int baud, int dataBit, int stopBit, int parity) {
	int res = 0;
	struct termios	newtio;
	//int baud = 115200;
	//char data[512];

	/* �б�/���� ���� �� ��ġ�� ����.(O_RDWR)
     ������ ���� �ÿ� <CTRL>-C ���ڰ� ���� ���α׷��� ������� �ʵ���
     �ϱ� ���� controlling tty�� �ȵǵ��� �Ѵ�.(O_NOCTTY)
	*/
	fd = open(PORT_DEVICE, O_RDWR | O_NOCTTY );
	if (fd < 0)
	{
		fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_open() fail %d \n", fd);
		res = fd;
		return res;
	}

	/*
    BAUDRATE: ���� �ӵ�. cfsetispeed() �� cfsetospeed() �Լ��ε� ���� ����
    CRTSCTS : �ϵ���� �帧 ����.
    CS8     : 8N1 (8bit, no parity, 1 stopbit)
    CLOCAL  : Local connection. �� ��� ���� �ʴ´�.
    CREAD   : ���� ������ �����ϰ� �Ѵ�.
	CSTOPB2 : use 2 stopbit
	PARENB  : use parity
	*/
	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = get_baud(baud) | CLOCAL | CREAD;
	if (dataBit == 8)
		newtio.c_cflag |= CS8;
	else if (dataBit == 7)
		newtio.c_cflag |= CS7;
	if (stopBit == 2)
		newtio.c_cflag |= CSTOPB;
	if (parity == 1) //even parity
		newtio.c_cflag |= PARENB;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

		// poll ����� ���� �غ�

	poll_events.fd        = fd;
	poll_events.events    = POLLIN | POLLERR;          // ���ŵ� �ڷᰡ �ִ���, ������ �ִ���
	poll_events.revents   = 0;

	usleep(1000);

	fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_open() result : %d \n", res);

	thread_serial_end=0;
	thr_serial_id = pthread_create(&recvThread, NULL, serial_recv_thread, NULL);
	if(thr_serial_id < 0)
	{
		LOGE("serial recv Thread create error");
	}

	return res;
}

int sdcd_serial_close() {
	int res = 0;
	thread_serial_end=1;
	if(recvThread!=0)
	{
		pthread_join(recvThread,NULL);
		recvThread=0;
	}
	if (fd > 0)
		close(fd);

	return res;
}
