#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <pthread.h>
#include <termios.h>                  // B115200, CS8 �� ��� ����
#include <fcntl.h>                    // O_RDWR , O_NOCTTY ���� ��� ����

#include "../include/serial_dev_2.h"
#include "../include/global.h"
#include "../include/application.h"
#include "../include/ep.h"

#define EX_PORT_DEVICE	"/dev/ttyS3"


#define EX_PORT_READ_SIZE	256
#define EX_PORT_BUFF_SIZE	256

struct pollfd	ex_port_poll_events;      //üũ�� event ������ ���� struct
extern int pprf;	//231206 Modify
unsigned char ex_port_read_task_flag = 0;
int ex_port_fd = -1;
int ex_port_thr_serial_id;
int ex_port_recvCnt = 0;
char ex_port_recvBuf[EX_PORT_BUFF_SIZE] = {0};
unsigned char ex_port_com_buffer_recive[EX_PORT_BUFF_SIZE] = {0};
int ex_port_com_buffer_recive_cnt = 0;
int ex_port_com_buffer_cnt = 0;
unsigned char ex_port_com_put = 0;
char ex_port_rs232_read_flag;
int ex_port_com_put_check = 0;
unsigned  char ex_port_com_task_state = 0;
unsigned char ex_port_com_buffer[EX_PORT_BUFF_SIZE]  = {0};
unsigned int ex_port_data_cnt = 0;

unsigned char adc_sensing_ch = 0;

static int ex_port_get_baud(int baud)
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

int ex_port_serial_write(int len, char *cmdBuf) 
{	
	int res = 0;

	// fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_write() len:%d, cmd:%s\n", len, cmdBuf);
	res = write(ex_port_fd, cmdBuf, len);
	if (res < 0) {
		fprintf(stderr, "[sdcd-serial-dev_2.c] ex_port_serial_write() fail - len:%d, cmd:%s\n", len, cmdBuf);
		return res;
	}

	return res;
}

int ex_port_serial_open(int baud, int dataBit, int stopBit, int parity) 
{
	int res = 0;
	struct termios	ex_port_newtio;
	//int baud = 115200;
	//char data[512];

	/* �б�/���� ���� �� ��ġ�� ����.(O_RDWR)
     ������ ���� �ÿ� <CTRL>-C ���ڰ� ���� ���α׷��� ������� �ʵ���
     �ϱ� ���� controlling tty�� �ȵǵ��� �Ѵ�.(O_NOCTTY)
	*/
	ex_port_fd = open(EX_PORT_DEVICE, O_RDWR | O_NOCTTY );
	if (ex_port_fd < 0)
	{
		fprintf(stderr, "[sdcd-serial-dev_2.c] ex_port_serial_open() fail %d \n", ex_port_fd);
		res = ex_port_fd;
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
	memset(&ex_port_newtio, 0, sizeof(ex_port_newtio));
	ex_port_newtio.c_cflag = ex_port_get_baud(baud) | CLOCAL | CREAD;
	if (dataBit == 8)
		ex_port_newtio.c_cflag |= CS8;
	else if (dataBit == 7)
		ex_port_newtio.c_cflag |= CS7;
	if (stopBit == 2)
		ex_port_newtio.c_cflag |= CSTOPB;
	if (parity == 1) //even parity
		ex_port_newtio.c_cflag |= PARENB;
	ex_port_newtio.c_oflag = 0;
	ex_port_newtio.c_lflag = 0;
	ex_port_newtio.c_cc[VTIME] = 0;
	ex_port_newtio.c_cc[VMIN] = 1;

	tcflush(ex_port_fd, TCIFLUSH);
	tcsetattr(ex_port_fd, TCSANOW, &ex_port_newtio);

		// poll ����� ���� �غ�

	ex_port_poll_events.fd        = ex_port_fd;
	ex_port_poll_events.events    = POLLIN | POLLERR;          // ���ŵ� �ڷᰡ �ִ���, ������ �ִ���
	ex_port_poll_events.revents   = 0;

	usleep(1000);
	fprintf(stderr, "[serial-dev_2.c] ex_port_serial_open() result : %d \n", res);
	return res;
}

//void *ex_port_serial_recv_thread(void *data) 
void ex_port_serial_task(void) 
{
	int ex_port_poll_state;
	int cnt = 0;
	int ex_port_i=0;
	int packet_end=0;
	char ex_port_readbuff;
	int j;
	int t, dt;
	int adc_start = 0;	
	int adc_end = 0;	
	int adc_time = EX_UART_RX_TIMEOUT;	

	ex_port_recvCnt = 0;
	memset (ex_port_recvBuf, 0, EX_PORT_BUFF_SIZE);
	ex_port_read_task_flag = 1;
	adc_start = timeout_msclock();	
	while (ex_port_read_task_flag)
	{
		ex_port_poll_state = poll(                                 // poll()�� ȣ���Ͽ� event �߻� ���� Ȯ��
			(struct pollfd*)&ex_port_poll_events,  // event ��� �����
			1,  // üũ�� pollfd ����
			0	// time out �ð�
		);
		// fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_recv() in 2\n");
		if (ex_port_poll_state > 0)                             // �߻��� event �� ����
		{
			if (ex_port_poll_events.revents & POLLIN)            // event �� �ڷ� ����?
			{
				read(ex_port_fd, &ex_port_readbuff, 1);	
				ex_port_com_buffer_recive[ex_port_com_buffer_recive_cnt++] = ex_port_readbuff;
				//printf("%x", ex_port_readbuff);
				//printf("readbuff = %c / %x\r\n", readbuff,readbuff);
				if(ex_port_com_buffer_recive_cnt >= EX_PORT_BUFFER_SIZE) ex_port_com_buffer_recive_cnt = 0;
			
				if (ex_port_poll_events.revents & POLLERR)      // event �� ����?
				{					
					fprintf(stderr, "[serial-dev_2.c] ex_port_serial_recv() poll_events error!\n");
				//break;
				}
				
			}
			else if (ex_port_poll_state < 0) {
				fprintf(stderr, "[serial-dev_2.c] ex_port_serial_recv() Critial Error!\n");
				packet_end=1;
				break;
			}
			if(ex_port_read_task_flag==0) break;
		}
		if(ex_port_com_buffer_recive_cnt != ex_port_com_buffer_cnt)
		{		
			//printf("ex_port_com_buffer_recive[%d] = %x / ex_port_com_buffer_cnt = %d\r\n", ex_port_com_buffer_recive_cnt, ex_port_com_buffer_recive[ex_port_com_buffer_recive_cnt],ex_port_com_buffer_cnt);		
			ex_port_rs232_read_flag = 1;
			ex_port_i=0;
			ex_port_com_put = ex_port_com_buffer_recive[ex_port_com_buffer_cnt++];
			ex_port_data_check();
			if(ex_port_com_buffer_cnt >= EX_PORT_BUFFER_SIZE) ex_port_com_buffer_cnt = 0;		
		}
		if(ex_port_rs232_read_flag)
		{
			if(ex_port_i > 300)
			{
				ex_com_read_data_init();
				ex_port_i = 0;
				ex_port_rs232_read_flag = 0;
				ex_port_read_task_flag = 0;		
				return;		
			}
			ex_port_i++;
		}				
		usleep(100);
		adc_end = timeout_msclock() - adc_start;	
		if(adc_end >= adc_time) break;		
	}
	//return NULL;
}

void ex_com_read_data_init(void)
{
   memset(&ex_port_com_buffer_recive, 0, sizeof(ex_port_com_buffer_recive));	
   ex_port_com_put_check = 0;
   ex_port_com_task_state = 0;
}

void ex_port_data_check(void)
{
	ex_port_com_put_check = (ex_port_com_put_check << 8) + (ex_port_com_put&0xff);	
	if(((ex_port_com_put_check&0xffffffff) == EX_PORT_STX))
	{
		ex_port_com_task_state = 1;
		ex_port_data_cnt = 0;
		memset(ex_port_com_buffer, 0, sizeof(ex_port_com_buffer));
	}	
	else if((ex_port_com_put_check == EX_PORT_ETX) && (ex_port_com_task_state == 1))
	{
		ex_port_rs232_read_flag = 0;
		ex_port_com_task_state = 0;
		ex_port_com_buffer[ex_port_data_cnt--] = 0;
		ex_port_com_buffer[ex_port_data_cnt--] = 0;
		ex_port_com_buffer[ex_port_data_cnt--] = 0;
		ex_port_com_buffer[ex_port_data_cnt] = 0;
		ex_port_com_task(ex_port_com_buffer);		
	}
	else
	{		
		if(ex_port_com_task_state == 1) ex_port_com_buffer[ex_port_data_cnt++] = ex_port_com_put;
		if(ex_port_data_cnt >= COM_BUFFER_SIZE) ex_port_com_task_state = 0;
	}	
}

static void ex_port_com_task(char *ptr)
{
	int i = 0;
	unsigned char ex_port_checksum = 0;
	unsigned char ex_port_rx_checksum = 0;
	unsigned char data_cnt = 0;

	data_cnt = ex_port_data_cnt-EX_PORT_CHECKSUM_CNT;

	for(i=0; i<data_cnt; i++)
	{
		ex_port_checksum ^= ex_port_com_buffer[i];
	}
	ex_port_rx_checksum = ex_port_com_buffer[data_cnt];
	//printf("ex_port_rx_checksum  = %x / ex_port_com_buffer[data_cnt] = %x\r\n", ex_port_rx_checksum, ex_port_com_buffer[data_cnt]);
	if(ex_port_checksum != ex_port_rx_checksum)
	{
		printf("EX_PORT_CHECKSUM_ERROR\r\n");
	}
	else
	{
		ex_port_read_task_flag = 0;
		if((ex_port_com_buffer[0] == EX_ADC_SEN) && (ex_port_com_buffer[1] == EX_ADC_SEN_LEN) && (pg_on_flag == PG_OFF))
		{
			unsigned char ex_send_data[10] = {0,};
			extern unsigned char adc_sensing_ch;
			if(ex_port_com_buffer[2] == EX_OK_ACK) 
			{
				power_off();
				ex_send_data[0] = EX_ADC_DATA_CHECK;	
				ex_send_data[1] = adc_sensing_ch;					
				ex_port_send_task(ex_send_data,EX_ADC_DATA_CHECK_LEN);	
				ex_port_serial_task();			
			}
		}
		else if((ex_port_com_buffer[0] == EX_ADC_DATA_CHECK) && (ex_port_com_buffer[1] == EX_ADC_DATA_CHECK_REQUEST_LEN) && (pg_on_flag == PG_OFF))
		{
			unsigned char index = 0;
			unsigned char ex_output_vol_adc_result[EX_PORT_OUTPUT_CNT];

			char data[100] = {0};					
			index = ex_port_com_buffer[2];
			if(index == 0)	
			{				
				memset(&ex_output_vol_adc_result, 0, sizeof(ex_output_vol_adc_result));
			}	
			ex_output_vol_adc_result[index] =	ex_port_com_buffer[3];
			ex_adc_total_result[index] = ex_output_vol_adc_result[index] | ex_input_vol_adc_result[index];
			printf("ex_adc_total_result[%d] = %d / ex_input_vol_adc_result[%d] = %d / ex_output_vol_adc_result[%d] = %x\r\n", index, ex_adc_total_result[index], index, ex_input_vol_adc_result[index], index, ex_output_vol_adc_result[index]);
			if(index <= EX_LPSPARE1)
			{
				index++;	
				if(index <= EX_LPSPARE1)	ex_vol_set_task(index);   		
				else	power_off(); 	 								
			}		
		}
		else if((ex_port_com_buffer[0] == AUTO_CAL_TASK) && (ex_port_com_buffer[1] == AUTO_CAL_ACK_LEN))
		{
			auto_cal_data = 0;
			auto_cal_data_float = 0;
			unsigned char ex_recive_data_delay_task_state = 0;
			auto_cal_data |= (ex_port_com_buffer[2] << 24);	
			auto_cal_data |= (ex_port_com_buffer[3] << 16);	
			auto_cal_data |= (ex_port_com_buffer[4] << 8);	
			auto_cal_data |= (ex_port_com_buffer[5] << 0);	
			auto_cal_data_float = ((double)auto_cal_data / 1000000);
		}	
		else if((ex_port_com_buffer[0] == VOL_CUR_SELECT) && (ex_port_com_buffer[1] == VOL_CUR_SELECT_LEN))
		{
			vol_cur_select_flag = ex_port_com_buffer[2];
		}					
	} 
}


void ex_port_ack(void)
{
    char data[8] = {0};
    data[0] = 0x02;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x02;
    data[4] = 0x15;
    data[5] = 0x00;
    data[6] = 0x97;
    data[7] = 0x03;	
    ex_port_serial_write(8,data);
}

void ex_port_send_task(unsigned char *cmd, unsigned int len)
{
	int i = 0;
	int j = 0;
	int l = 0;
	int data_len = 0;
	char ex_port_str[EX_PORT_BUFF_SIZE]={0};
	unsigned char ex_port_checksum=0;
	unsigned short ex_port_tx_len = 0;	
	ex_port_str[0] = 0x37;
	ex_port_str[1] = 0xa4;
	ex_port_str[2] = 0xc2;
	ex_port_str[3] = 0x95;
	ex_port_str[4] = len;

	data_len =	len+EX_STX_LEN+EX_LEN_LEN;
	for(i = EX_STX_LEN+EX_LEN_LEN; i < data_len ; i++)
	{
		ex_port_str[i] = cmd[ex_port_tx_len++];
		//printf("i = %d / ex_port_str[%d] = %x / len = %d\r\n", i,i, ex_port_str[i],len);
	}

	for(i=EX_STX_LEN; i<data_len; i++)	ex_port_checksum ^= *(ex_port_str+i);
	ex_port_str[i++] = ex_port_checksum;
	ex_port_str[i++] = 0x59;
	ex_port_str[i++] = 0x2c;
	ex_port_str[i++] = 0x4a;
	ex_port_str[i++] = 0x0d;
	ex_port_serial_write(i,ex_port_str);
	memset(&ex_port_str, 0, sizeof(ex_port_str));		
}
