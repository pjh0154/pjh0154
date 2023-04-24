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
#include "../include/application.h"
#include "../include/ep.h"
#include "../include/pca9665.h"
//#define PORT_DEVICE	"/dev/ttyPS3"
#define PORT_DEVICE	"/dev/ttyPS4"

#define READ_SIZE	256
#define BUFF_SIZE	256
#define CMD_TIMEOUT		2000	//1000 msec
#define SET_TIMEOUT		100
extern int cprf;
extern int aprf;
unsigned char com_buffer_recive[COM_BUFFER_SIZE] = {0};
int com_buffer_recive_cnt = 0;
int com_buffer_cnt = 0;
int com_buffer_recive_cnt_test = 0;
unsigned char com_buffer[COM_BUFFER_SIZE] = {0};
unsigned char length_buffer[2];
static unsigned short com_put_cnt = 0;
static unsigned char com_packet_state = 0;
unsigned char com_task_state;
unsigned char length_cnt = 0;
unsigned int recipe_download_cnt = 0;
unsigned int fw_fpga_download_cnt = 0;
int com_cnt = 0;
int com_read_cnt = 0;
unsigned char recipe_download_index = 0;
unsigned char fw_fpga_download_index = 0;
unsigned char pat_index = 0;
unsigned char pat_string_index[1] = {0};
int thr_serial_id;
int fd = -1;
char recvBuf[BUFF_SIZE];
int recvCnt = 0;
unsigned int com_put_check = 0;
extern unsigned char com_put;
unsigned char checksum = 0x00;
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
	char readbuff;
	int j;
	char key_flag = 0;
	int t, dt;

	recvCnt = 0;
	memset (readBuf, 0, READ_SIZE);
	memset (recvBuf, 0, BUFF_SIZE);

	while (!thread_serial_end)
	{
		poll_state = poll(                                 // poll()�� ȣ���Ͽ� event �߻� ���� Ȯ��
			(struct pollfd*)&poll_events,  // event ��� �����
			1,  // üũ�� pollfd ����
			0	// time out �ð�
		);
		// fprintf(stderr, "[sdcd-serial-dev.c] sdcd_serial_recv() in 2\n");
		if (poll_state > 0)                             // �߻��� event �� ����
		{

			if (poll_events.revents & POLLIN)            // event �� �ڷ� ����?
			{
				read(fd, &readbuff, 1);	
				com_buffer_recive[com_buffer_recive_cnt++] = readbuff;
				if(com_buffer_recive_cnt >= COM_BUFFER_SIZE) com_buffer_recive_cnt = 0;
			
				/*cnt = read(fd, readBuf, READ_SIZE);
				if (cnt > 0) {
					if(strncasecmp(readBuf,"[00]", 4)==0)			
					{
						if(key_flag)	
						{
							key_flag = 0;
							recipe_key_load("KEY1");
						}
						else
						{
							key_flag = 1;
							recipe_key_load("KEY0");
						}
					}
			

					//for(j=0; j<cnt; j++) printf("%c    %x\r\n", readBuf[j], readBuf[j]);
					//for(j=0; j<cnt; j++) printf("%c   \r\n", readBuf[j]);
					
					//LOGD("[%s]%s\n",__func__ ,readBuf);

				}*/
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
		if(com_buffer_recive_cnt != com_buffer_cnt)
		{				
			rs232_read_flag = 1;
			i=0;
			com_put = com_buffer_recive[com_buffer_cnt++];
			data_check();
			if(com_buffer_cnt >= COM_BUFFER_SIZE) com_buffer_cnt = 0;			
		}
		if(rs232_read_flag)
		{
			//if(i==0)	t = msclock();
			if(i > 300)
			{
				i = 0;
				rs232_read_flag = 0;				
				com_read_data_init();
				//dt = msclock() - t;	
				//printf("rs232_rx_ng = %d\r\n", dt);
			}
			i++;
		}				
	usleep(1);
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

void data_check(void)
{
	com_put_check = (com_put_check << 8) + (com_put&0xff);
	if(((com_put_check&0xff00ff) == 0x020080) && (com_task_state ==0))
	{
		recipe_download_cnt = (com_put_check&0x00ff00)>>8;
		com_task_state = 1;
		length_cnt = 0;
	}
	if(com_task_state == 1)
	{
		length_buffer[length_cnt] = com_put;
		if(length_cnt == 1)
		{
			com_cnt = length_buffer[1];
			com_read_cnt = 0;
			com_task_state = 2;
			checksum = length_buffer[0]^length_buffer[1]^recipe_download_cnt;
		}
		length_cnt++;			
	}
	else if(com_task_state == 2)
	{	
		com_buffer[com_read_cnt] = com_put;
		if(com_read_cnt ==0) recipe_download_index = com_buffer[com_read_cnt];
		//printf("tp = %x / %d\r\n", com_buffer[com_read_cnt], com_cnt);
		if((recipe_download_index == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_LEN))
		{
			if(com_read_cnt <= (RECIPE_DOWNLOAD_SIZE+CHECKSUM_or_ETX))
			{			
				com_buffer[com_read_cnt] = com_put;
				if((com_read_cnt == (RECIPE_DOWNLOAD_SIZE+CHECKSUM_or_ETX)) && (com_buffer[RECIPE_DOWNLOAD_SIZE+CHECKSUM_or_ETX]== 0x03))
				{
					com_task();
					com_task_state = 0;
					rs232_read_flag = 0;
				}			
			}
		}
		else if((recipe_download_index == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_LEN))
		{
			if(com_read_cnt <= (FW_FPGA_DOWNLOAD_SIZE+CHECKSUM_or_ETX))
			{			
				com_buffer[com_read_cnt] = com_put;
				if((com_read_cnt == (FW_FPGA_DOWNLOAD_SIZE+CHECKSUM_or_ETX)) && (com_buffer[FW_FPGA_DOWNLOAD_SIZE+CHECKSUM_or_ETX]== 0x03))
				{
					com_task();
					com_task_state = 0;
					rs232_read_flag = 0;
				}			
			}
		}
		else
		{
			if(com_read_cnt <= (com_cnt+CHECKSUM_or_ETX-1))
			{		
				com_buffer[com_read_cnt] = com_put;
				if((com_read_cnt == (com_cnt+CHECKSUM_or_ETX-1)) && (com_buffer[com_cnt+CHECKSUM_or_ETX-1]== 0x03))
				{
					com_task();
					com_task_state = 0;
					rs232_read_flag = 0;
				}				
			}
		}
		com_read_cnt++;				
	}	
}

void com_task(void)
{
	int i =0;
	unsigned char rx_checksum = 0;
	if((recipe_download_index == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_LEN))
	{
		{
		for(i=0; i<RECIPE_DOWNLOAD_USER_DATA_SIZE; i++)
			checksum ^= com_buffer[i];
		}				
	}
	else if((recipe_download_index == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_LEN))
	{
		{
		for(i=0; i<FW_FPGA_DOWNLOAD_USER_DATA_SIZE; i++)
			checksum ^= com_buffer[i];
		}				
	}
	else
	{		
		for(i=0; i<com_cnt; i++)
		{
			checksum ^= com_buffer[i];
		}
	}
	if((recipe_download_index == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_LEN))	rx_checksum = com_buffer[RECIPE_DOWNLOAD_USER_DATA_SIZE];
	else if((recipe_download_index == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_LEN))	rx_checksum = com_buffer[FW_FPGA_DOWNLOAD_USER_DATA_SIZE];
	else rx_checksum = com_buffer[com_cnt];	
	if(checksum != rx_checksum)
	{
		if((recipe_download_index == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_LEN))
		{
			printf("RX Checksum = 0x%02x\r\n", com_buffer[RECIPE_DOWNLOAD_USER_DATA_SIZE]);
			printf("Cal Checksum = 0x%02x\r\n", checksum);				
		}
		else if((fw_fpga_download_index == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_LEN))
		{
			printf("RX Checksum = 0x%02x\r\n", com_buffer[FW_FPGA_DOWNLOAD_USER_DATA_SIZE]);
			printf("Cal Checksum = 0x%02x\r\n", checksum);				
		}
		else
		{
			printf("RX Checksum = 0x%02x\r\n", com_buffer[com_cnt]);
			printf("Cal Checksum = 0x%02x\r\n", checksum);	
		}								
	}
	else
	{
		//printf("com_buffer[0] = %x / com_cnt = %d\r\n", com_buffer[0], com_cnt);
		if((com_buffer[0] == TURN_OFF) && (com_cnt == TURN_OFF_LEN) && (pg_on_flag == PG_ON))
		{
			pg_on_flag = 0;
			recipe_funtion_load("OFF");
			if(cprf)	printf("TURN_OFF_ok\r\n");
			goto ACK;
		}	
		else if((com_buffer[0] == TURN_ON) && (com_cnt == TURN_ON_LEN) && (pg_on_flag == PG_OFF))
		{
			//pg_on_flag = 1;
			//pat_index = 0;
			//sprintf(pat_string_index, "%d",pat_index);
			//recipe_funtion_load(pat_string_index);
			//if(cprf)	printf("TURN_ON_ok = %s\r\n", pat_string_index);

			pg_on_flag = 1;
			pat_index = 0;
			if(zone_select == 0x01)	sprintf(pat_string_index, "T%d",pat_index);
			else if(zone_select == 0x02)	sprintf(pat_string_index, "H%d",pat_index);
			else if(zone_select == 0x03)	sprintf(pat_string_index, "M%d",pat_index);
			else sprintf(pat_string_index, "%d",pat_index);
			if(cprf)	printf("ZONE = [%d]TURN_ON_ok = [%s]\r\n", zone_select, pat_string_index);
			recipe_funtion_load(pat_string_index);
			goto ACK;
		}
		else if((com_buffer[0] == NEXT_PATTERN) && (com_cnt == NEXT_PATTERN_LEN)&& (pg_on_flag == PG_ON))
		{
			//if(pat_index >= srecipe.recipe_func.func_cnt-1)	pat_index = 0;
			//else pat_index++;
			//sprintf(pat_string_index, "%d",pat_index);
			//recipe_funtion_load(pat_string_index);
			//if(cprf)	printf("TURN_ON_ok = %s\r\n", pat_string_index);
			recipe_funtion_load("OFF");
			if(zone_select == 0x01)
			{
				if(pat_index >= t_count-1)	pat_index = 0;	
				else	pat_index++; 
				sprintf(pat_string_index, "T%d",pat_index);
			}
			else if(zone_select == 0x02)
			{
				if(pat_index >= h_count-1)	pat_index = 0;	
				else	pat_index++; 
				sprintf(pat_string_index, "H%d",pat_index);				
			}
			else if(zone_select == 0x03)
			{
				if(pat_index >= m_count-1)	pat_index = 0;	
				else	pat_index++;
				sprintf(pat_string_index, "M%d",pat_index); 				
			}
			else 
			{
				if(pat_index >= n_count-2)	pat_index = 0;
				else pat_index++;	
				sprintf(pat_string_index, "%d",pat_index);			
			}
			if(cprf)	printf("ZONE = [%d]TURN_ON_ok = [%s]\r\n", zone_select, pat_string_index);
			recipe_funtion_load(pat_string_index);			
			goto ACK;
		}
		else if((com_buffer[0] == PREVIOUS_PATTERN) && (com_cnt == PREVIOUS_PATTERN_LEN)&& (pg_on_flag == PG_ON))
		{
			//if(pat_index == 0)	pat_index = srecipe.recipe_func.func_cnt-1;	
			//else pat_index--;
			//sprintf(pat_string_index, "%d",pat_index);
			//recipe_funtion_load(pat_string_index);
			recipe_funtion_load("OFF");
			if(zone_select == 0x01)
			{
				if(pat_index == 0)	pat_index = t_count-1;
				else 	pat_index--;
				sprintf(pat_string_index, "T%d",pat_index); 	
			}
			else if(zone_select == 0x02)
			{
				if(pat_index == 0)	pat_index = h_count-1;
				else 	pat_index--;
				sprintf(pat_string_index, "H%d",pat_index); 					
			}
			else if(zone_select == 0x03)
			{
				if(pat_index == 0)	pat_index = m_count-1;
				else 	pat_index--;
				sprintf(pat_string_index, "M%d",pat_index); 					
			}
			else
			{
				if(pat_index == 0)	pat_index = n_count-1;	
				else pat_index--;
				sprintf(pat_string_index, "%d",pat_index);				
			}					
			if(cprf)	printf("ZONE = [%d]TURN_ON_ok = [%s]\r\n", zone_select, pat_string_index);
			recipe_funtion_load(pat_string_index);				
			goto ACK;
		}
		else if((com_buffer[0] == RESISTANCE_REQUEST) && (com_cnt == RESISTANCE_REQUEST_LEN))
		{
			resistance_measurement_1();
			gpio->GPIO_DATA = 0x100;
			//gpio_reg_init();
			if(cprf) printf("reg_value_r = %.4fohm\r\n", (float)adc_res_value_r_1/10000);
			if(cprf) printf("reg_value_l = %.4fohm\r\n", (float)adc_res_value_l_1/10000); 				
			com_task_ack(RESISTANCE_REQUEST_ACK);
			goto DEFLAT;
		}
		else if((com_buffer[0] == SENSING_DATA_REQUEST) && (com_cnt == SENSING_DATA_REQUEST_LEN)&& (pg_on_flag == PG_ON))
		//else if((com_buffer[0] == SENSING_DATA_REQUEST) && (com_cnt == SENSING_DATA_REQUEST_LEN))		
		{
			ADC_AUTO_DATA_READ();
			if(aprf) ADC_DATA_PRINT();
			com_task_ack(SENSING_DATA_REQUEST_ACK);
			goto DEFLAT;
		}
		else if((com_buffer[0] == RGB_VOL_SET_REQUEST) && (com_cnt == RGB_VOL_SET_REQUEST_LEN)&& (pg_on_flag == PG_ON))		
		{			
			
			char index;		
			if(com_buffer[1] == 0x00)	i2c_com_bist_pattern("W",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x01)	i2c_com_bist_pattern("R",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x10)	i2c_com_bist_pattern("G",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x11)	i2c_com_bist_pattern("B",com_buffer[2],FULL_COLOR_MODE1);  
		}		
		else if((com_buffer[0] == GMA_BLOCK_REQUEST) && (com_cnt == GMA_BLOCK_REQUEST_LEN)&& (pg_on_flag == PG_ON))		
		{
			unsigned short reg_addr = 0;
			unsigned short rgb_d[3] = {0};
			if(com_buffer[1] == _0)	reg_addr =	_0_ADDR;
			else if(com_buffer[1] == TOP)	reg_addr =	TOP_ADDR;
			else if(com_buffer[1] == BOT)	reg_addr =	BOT_ADDR;
			else if(com_buffer[1] == PRE)	reg_addr =	PRE_ADDR;
			rgb_d[0] = com_buffer[2] << 8;
			rgb_d[0] = rgb_d[0] | (com_buffer[3]&0x00ff);
			rgb_d[1] = com_buffer[4] << 8;
			rgb_d[1] = rgb_d[1] | (com_buffer[5]&0x00ff);	
			rgb_d[2] = com_buffer[6] << 8;
			rgb_d[2] = rgb_d[2] | (com_buffer[7]&0x00ff);				
					
			i2c_com_gma_block(reg_addr, rgb_d);
		}
		else if((com_buffer[0] == VREFH_VREFL_REQUEST) && (com_cnt == VREFH_VREFL_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			unsigned char slave_address = 0xa0>>1;
			unsigned short reg_addr = 0;
			int write_byte = 1;
			if(com_buffer[1] == VREFH)	reg_addr = VREFH_ADDR;
			else if(com_buffer[1] == VREFL)	reg_addr = VREFL_ADDR;

			i2c_write(slave_address,reg_addr,&com_buffer[2],write_byte);
		}		
		else if((com_buffer[0] == ZONE_SELECT) && (com_cnt == ZONE_SELECT_REQUEST_LEN))
		{
			zone_select = com_buffer[1]; 	
			if(cprf)	printf("ZONE = %x\r\n", zone_select);		
		}	
		else if(com_buffer[0] == I2C_WRITE_REQUEST)
		{
			unsigned char slave_address = 0xa0>>1;
			int reg_address = 0;
			int write_byte = 0;
			int i = 0;
			char write_buffer[10] = {0};  			
			write_byte = com_cnt - 3;
			reg_address = (com_buffer[1] << 8) | com_buffer[2];

			if(reg_address >= 0x1000)	slave_address = 0xa2>>1;

			for(i = 0 ; i < write_byte ; i++)
			{
				write_buffer[i] = com_buffer[3+i];	
			}
			i2c_write(slave_address,reg_address,write_buffer,write_byte);				
		}
		else if((com_buffer[0] == I2C_READ_REQUEST) && (com_cnt == I2C_READ_REQUEST_LEN))
		{
			unsigned char slave_address = 0xa1>>1;
			int read_byte = 0;
			int reg_address = 0;
			int i = 0;
			char read_buffer[10] = {0};
			char str[20]={0};
			unsigned short checksum=0;
			unsigned short tx_len = 0;		

			read_byte = com_buffer[3];
			reg_address = (com_buffer[1] << 8) | com_buffer[2];

			if(reg_address >= 0x1000)	slave_address = 0xa3>>1;

			i2c_read(slave_address,reg_address,read_buffer,read_byte);

			str[tx_len++] = 0x02;
			str[tx_len++] = 0x00;
			str[tx_len++] = 0x80;	
			str[tx_len++] = I2C_READ_ACK_LEN;
			str[tx_len++] = I2C_READ_ACK;	
			str[tx_len++] = com_buffer[1];
			str[tx_len++] = com_buffer[2];		

			for(i = 0 ; i < read_byte ; i++)
			{
				str[tx_len++] = read_buffer[i];
			}
			if(read_byte <4)
			{
				if(read_byte == 1)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;					
				}
				else if(read_byte == 2)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;					
				}
				else if(read_byte == 3)
				{
					str[tx_len++] = 0x00;	
				}
			}
			if(cprf)
			{
				printf("SLAVE_ADDRESS = %x\r\n", slave_address);
				printf("REG_ADDRESS = %x\r\n", reg_address);
				printf("read_byte = %x\r\n", read_byte);
				printf("address = %x\r\n", com_buffer[1]);
				printf("address = %x\r\n", com_buffer[2]); 
				printf("read_buffer[2] = %x\r\n", read_buffer[0]); 
				printf("read_buffer[3] = %x\r\n", read_buffer[1]);
				printf("read_buffer[4] = %x\r\n", read_buffer[2]); 
				printf("read_buffer[4] = %x\r\n", read_buffer[3]);         
			} 
			for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
			str[tx_len++] = checksum;
			str[tx_len++] = 0x03;
			sdcd_serial_write(tx_len, str);
			memset(&str, 0, sizeof(str));						
		}					
		else if((com_buffer[0] == FW_FPGA_VER_REQUEST) && (com_cnt == FW_FPGA_VER_REQUEST_LEN))
		{
			com_task_ack(FW_FPGA_VER_REQUEST);
		}			
		else if((com_buffer[0] == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_START_LEN) && (pg_on_flag == PG_OFF))
		{
            fw_fpga.download_ch = 0;
            fw_fpga.download_packetnumber = 0;
            fw_fpga.download_mode = 1;
            fw_fpga.download_length = (com_buffer[1] << 24)|(com_buffer[2] << 16)|(com_buffer[3] << 8)|(com_buffer[4] << 0);
            if(cprf)	printf("CH[%02d] FW_FPGA Download Length = %d\r\n", fw_fpga.download_ch, fw_fpga.download_length);	
			goto ACK;	
		}
		else if((com_buffer[0] == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_LEN) && (pg_on_flag == PG_OFF))
		{
            if(fw_fpga.download_mode != 1)
			{
                if(cprf)	printf("CH[%02d] No Download Mode\r\n", fw_fpga.download_ch);
                goto NACK;
			}
            if(recipe_download_cnt != (fw_fpga.download_packetnumber & 0xFF))
			{
                if(cprf)	printf("CH[%02d] Packet Numer Error (Recv 0x%02x, Current 0x%02x)\r\n", fw_fpga.download_ch, recipe_download_cnt, (fw_fpga.download_packetnumber&0xFF));
                goto NACK;
            }
            FILE *model_file = NULL;
            if(fw_fpga.download_packetnumber == 0)
			{
                if(access("/f0/fw_fpga", F_OK)) system("mkdir /f0/fw_fpga");
                model_file = fopen(FW_FPGA_FILE_PATH, "w+b");
            }
			else model_file = fopen(FW_FPGA_FILE_PATH, "ab");
            fwrite(&com_buffer[1],(fw_fpga.download_length >= 1024) ? 1024 : fw_fpga.download_length,1,model_file);
            fclose(model_file);
			if(cprf)	printf("fw_fpga File write, Packet Num = %d\r\n", fw_fpga.download_packetnumber);	
			fw_fpga.download_packetnumber++;
			fw_fpga.download_length -= (fw_fpga.download_length >= 1024) ? 1024 : fw_fpga.download_length;
			uart_ack();
            if(fw_fpga.download_length == 0)
			{
				char temp_str[64] = {0};
				printf("fw_fpga_download_ok\r\n");	
                fw_fpga.download_mode = 0;
                sprintf(temp_str, "unzip -o %s -d %s", FW_FPGA_FILE_PATH, "/home/root/");
                system(temp_str);
                memset(temp_str, 0, sizeof(temp_str));
                sprintf(temp_str, "rm %s", FW_FPGA_FILE_PATH);
                system(temp_str);
		        system("reboot");				
                system("sync");
            }			
		}			
		else if((com_buffer[0] == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_START_LEN) && (pg_on_flag == PG_OFF))
		{
            recipe.download_ch = 0;
            recipe.download_packetnumber = 0;
            recipe.download_mode = 1;
            recipe.download_length = (com_buffer[1] << 24)|(com_buffer[2] << 16)|(com_buffer[3] << 8)|(com_buffer[4] << 0);
            if(cprf)	printf("CH[%02d] Recipe Download Length = %d\r\n", recipe.download_ch, recipe.download_length);	
			goto ACK;	
		}
		else if((com_buffer[0] == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_LEN) && (pg_on_flag == PG_OFF))
		{
            if(recipe.download_mode != 1)
			{
                if(cprf)	printf("CH[%02d] No Download Mode\r\n", recipe.download_ch);
                goto NACK;
			}
            if(recipe_download_cnt != (recipe.download_packetnumber & 0xFF))
			{
                if(cprf)	printf("CH[%02d] Packet Numer Error (Recv 0x%02x, Current 0x%02x)\r\n", recipe.download_ch, recipe_download_cnt, (recipe.download_packetnumber&0xFF));
                goto NACK;
            }
            FILE *model_file = NULL;
            if(recipe.download_packetnumber == 0)
			{
                if(access("/f0/recipe", F_OK)) system("mkdir /f0/recipe");
                model_file = fopen(RECIPE_FILE_PATH, "w+b");
            }
			else model_file = fopen(RECIPE_FILE_PATH, "ab");
            fwrite(&com_buffer[1],(recipe.download_length >= 1024) ? 1024 : recipe.download_length,1,model_file);
            fclose(model_file);
			if(cprf)	printf("Recipe File write, Packet Num = %d\r\n", recipe.download_packetnumber);	
			recipe.download_packetnumber++;
			recipe.download_length -= (recipe.download_length >= 1024) ? 1024 : recipe.download_length;
			uart_ack();
            if(recipe.download_length == 0)
			{
				printf("Recipe_download_ok\r\n");	
				model_recipe_read();
                recipe.download_mode = 0;
                system("sync");
            }		
		}
		else if((com_buffer[0] == RESET_REQUEST) && (com_cnt == RESET_REQUEST_LEN) && (pg_on_flag == PG_OFF))
		{
			printf("*******************************************REBOOT************************************************************\r\n");
			system("reboot");			
		}	
		else ("COMMAND NOT FOUND\r\n");		

		ACK:
		uart_ack();
		return;
		NACK:
		uart_nack();
		return;	
		DEFLAT:
		return;		
	}
	checksum = 0;	
}

void com_read_data_init(void)
{
   memset(&com_buffer, 0, sizeof(com_buffer));	
   memset(&length_buffer, 0, sizeof(length_buffer));	
   com_put_check = 0;
   com_task_state = 0;
}

void uart_ack(void)
{
    char data[8] = {0};
    data[0] = 0x02;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x02;
    data[4] = 0x06;
    data[5] = 0x00;
    data[6] = 0x84;
    data[7] = 0x03;	
    sdcd_serial_write(8,data);
}
void uart_nack(void)
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
    sdcd_serial_write(8,data);
}

void com_task_ack(unsigned char cmd)
{
	int i = 0;
	int j = 0;
	char str[COM_BUFFER_SIZE]={0};
	unsigned short checksum=0;
	unsigned short tx_len = 0;	


	if(cmd == RESISTANCE_REQUEST_ACK)
	{
		str[tx_len++] = 0x02;
		//str[tx_len++] = 0x87;
		str[tx_len++] = 0x90;
		str[tx_len++] = 0x80;
		str[tx_len++] = RESISTANCE_REQUEST_ACK_LEN;
		//str[tx_len++] = RESISTANCE_REQUEST_ACK;

		str[tx_len++] = (adc_res_value_r_1>>24)&0xff;
		str[tx_len++] = (adc_res_value_r_1>>16)&0xff;
		str[tx_len++] = (adc_res_value_r_1>>8)&0xff;
		str[tx_len++] = (adc_res_value_r_1)&0xff;

		str[tx_len++] = (adc_res_value_l_1>>24)&0xff;
		str[tx_len++] = (adc_res_value_l_1>>16)&0xff;
		str[tx_len++] = (adc_res_value_l_1>>8)&0xff;
		str[tx_len++] = (adc_res_value_l_1)&0xff;

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;	
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));		
	}
	else if(cmd == SENSING_DATA_REQUEST_ACK)
	{
		str[tx_len++] = 0x02;
		//str[tx_len++] = 0x87;
		str[tx_len++] = 0x92;
		str[tx_len++] = 0x80;		
		str[tx_len++] = SENSING_DATA_REQUEST_ACK_LEN;
		//str[tx_len++] = SENSING_DATA_REQUEST_ACK;
		for(j = 0 ; j < 40 ; j++)
		{
			str[tx_len++] = (adc_sensing_value[j]>>8)&0xff;			
			str[tx_len++] = adc_sensing_value[j]&0xff;			
		}

		//tx_len += SENSING_DATA_REQUEST_ACK_LEN-1;	
		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}
	else if(cmd == OCP_STATUS_ACK)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = OCP_STATUS_ACK;
		str[tx_len++] = 0x80;		
		str[tx_len++] = OCP_LEN;
		str[tx_len++] = 0x01;
		str[tx_len++] = 0x00;
		for(i=1; i<tx_len; i++) checksum ^= *(str+i);	
		str[tx_len++] = checksum;	
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));			
	}	
	else if(cmd == FW_FPGA_VER_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = FW_FPGA_VER_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = FW_FPGA_VER_REQUEST_LEN;
		str[tx_len++] = ep970_ver;	
		str[tx_len++] = ep970_ver;
		for(i=1; i<tx_len; i++) checksum ^= *(str+i);
		str[tx_len++] = checksum;	
		str[tx_len++] = 0x03;	
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));		
	}	
}

