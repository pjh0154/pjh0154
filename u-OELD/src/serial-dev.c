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
#include "../include/gpio-dev.h"
//#define PORT_DEVICE	"/dev/ttyPS3"
#define PORT_DEVICE	"/dev/ttyPS4"

#define READ_SIZE	256
#define BUFF_SIZE	256
#define CMD_TIMEOUT		2000	//1000 msec
#define SET_TIMEOUT		100
extern int cprf;
extern int aprf;
extern int dprf;
extern int nprf;
extern int vprf;
extern int pprf;	//231206 Modify
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
//unsigned char pat_string_index[5] = {0};
unsigned char pat_string_index[32] = {0};	//230929 Modify
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
				//printf("readbuff = %c / %x\r\n", readbuff,readbuff);
				if(com_buffer_recive_cnt >= COM_BUFFER_SIZE) com_buffer_recive_cnt = 0;
			
				/*cnt = read(fd, readBuf, READ_SIZE);
				if (cnt > 0) {
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
	//usleep(1);
	usleep(100);
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
	int elvss_4byte = 0;	//231013 Modify		
    short elvss_2byte = 0;	//231013 Modify
    int avdd_4byte = 0;		//231013 Modify
    short avdd_2byte = 0;	//231013 Modify	
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
		if(dprf)	printf("com_buffer[0] = %x / com_buffer[1] = %x / com_buffer[2] = %x / com_buffer[3] = %x /com_cnt = %d\r\n", com_buffer[0], com_buffer[1], com_buffer[2], com_buffer[3], com_cnt);
		//if((com_buffer[0] == TURN_OFF) && (com_cnt == TURN_OFF_LEN) && (pg_on_flag == PG_ON))
		if((com_buffer[0] == TURN_OFF) && (com_cnt == TURN_OFF_LEN))
		{
			//pg_on_flag = PG_OFF;
			recipe_funtion_load("OFF");
			if(cprf)	printf("TURN_OFF_ok\r\n");
			uart_ack();
			//uart_index_ack(TURN_OFF);
			//goto ACK;
		}	
		else if((com_buffer[0] == TURN_ON) && (com_cnt == TURN_ON_LEN) && (pg_on_flag == PG_OFF))
		{
			//cur_sensing_reset_flag = 1;	       //231013 Modify
			pg_on_flag = PG_ON;
			pat_index = 0;
			if(zone_select == 0x01)	sprintf(pat_string_index, "T%d",pat_index);
			else if(zone_select == 0x02)	sprintf(pat_string_index, "H%d",pat_index);
			else if(zone_select == 0x03)	sprintf(pat_string_index, "M%d",pat_index);
			else sprintf(pat_string_index, "%d",pat_index);
			if(cprf)	printf("ZONE = [%d]TURN_ON_ok = [%s] / STRLEN = %d / SIZEOF = %d\r\n", zone_select, pat_string_index, strlen(pat_string_index), sizeof(pat_string_index));
			if(system_load_flag==0)
			{
				memset(&error_data, 0, sizeof(error_data));
				recipe_system_load();
				system_load_flag = 1;
			}
			if(pg_on_flag != PG_OFF)	recipe_funtion_load(pat_string_index);
			else	system_load_flag = 0;
			if(pg_on_flag == PG_OFF)	system_load_flag = 0; 
			
			uart_ack();
			//uart_index_ack(TURN_ON);
			//goto ACK;
		}
		else if((com_buffer[0] == NEXT_PATTERN) && (com_cnt == NEXT_PATTERN_LEN)&& (pg_on_flag == PG_ON))
		{
			//cur_sensing_reset_flag = 1;	       //231013 Modify
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
			uart_ack();
			//uart_index_ack(NEXT_PATTERN);			
			//goto ACK;
		}
		else if((com_buffer[0] == PREVIOUS_PATTERN) && (com_cnt == PREVIOUS_PATTERN_LEN)&& (pg_on_flag == PG_ON))
		{
			//cur_sensing_reset_flag = 1;	       //231013 Modify
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
				if(pat_index == 0)	pat_index = n_count-2;	
				else pat_index--;
				sprintf(pat_string_index, "%d",pat_index);				
			}					
			if(cprf)	printf("ZONE = [%d]TURN_ON_ok = [%s]\r\n", zone_select, pat_string_index);
			recipe_funtion_load(pat_string_index);	
			uart_ack();	
			//uart_index_ack(PREVIOUS_PATTERN);			
			//goto ACK;
		}
		else if((com_buffer[0] == RESISTANCE_REQUEST) && (com_cnt == RESISTANCE_REQUEST_LEN))
		{
			resistance_measurement_1();
			gpio->GPIO_DATA = 0x100;	//231027 Modify 
			//gpio_reg_init();
			if(cprf) printf("reg_value_r = %.4fohm\r\n", (float)adc_res_value_r_1/10000);
			if(cprf) printf("reg_value_l = %.4fohm\r\n", (float)adc_res_value_l_1/10000); 				
			com_task_ack(RESISTANCE_REQUEST_ACK);
			//goto DEFLAT;
		}
		//else if((com_buffer[0] == SENSING_DATA_REQUEST) && (com_cnt == SENSING_DATA_REQUEST_LEN)&& (pg_on_flag == PG_ON))	
		else if((com_buffer[0] == SENSING_DATA_REQUEST) && (com_cnt == SENSING_DATA_REQUEST_LEN))	//231013 Modify	
		{
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
			ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
			elvss_2byte = fabs(elvss_cur_sensing_data*10);                
			ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
			avdd_2byte = fabs(avdd_cur_sensing_data*10);                
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);

			ADC_AUTO_DATA_READ();
			adc_sensing_value[22] = avdd_2byte;
			adc_sensing_value[32] = elvss_2byte;
			if(aprf) ADC_DATA_PRINT();
			com_task_ack(SENSING_DATA_REQUEST_ACK);  
		}
		else if((com_buffer[0] == RGB_VOL_SET_REQUEST) && (com_cnt == RGB_VOL_SET_REQUEST_LEN)&& (pg_on_flag == PG_ON))		
		{
			//cur_sensing_reset_flag = 1;	       //231013 Modify						
			char index;		
			if(com_buffer[1] == 0x00)	i2c_com_bist_pattern("W",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x01)	i2c_com_bist_pattern("R",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x10)	i2c_com_bist_pattern("G",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x11)	i2c_com_bist_pattern("B",com_buffer[2],FULL_COLOR_MODE1);  
			//goto DEFLAT;
		}		
		else if((com_buffer[0] == GMA_BLOCK_REQUEST) && (com_cnt == GMA_BLOCK_REQUEST_LEN)&& (pg_on_flag == PG_ON))		
		{
			//cur_sensing_reset_flag = 1;	       //231013 Modify
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
			//goto DEFLAT;
		}
		else if((com_buffer[0] == VREFH_VREFL_REQUEST) && (com_cnt == VREFH_VREFL_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			//cur_sensing_reset_flag = 1;		       //231013 Modify
			unsigned char slave_address = i2c_wr_addr>>1;
			unsigned short reg_addr = 0;
			int write_byte = 1;
			unsigned char write_buffer[10] = {0};
			if(com_buffer[1] == VREFH)	reg_addr = VREFH_ADDR;
			else if(com_buffer[1] == VREFL)	reg_addr = VREFL_ADDR;

			//if(com_buffer[2] > 0x1F) com_buffer[2] = 0x1F;
			//write_buffer[0] = 0x40 | (com_buffer[2]&0xff);
			write_buffer[0] = com_buffer[2] & 0xff;

			if(0 <= i2c_write(slave_address,reg_addr,&write_buffer[0],write_byte))
			{
				if(cprf)	printf("VREFH_VREFL_WRITE_OK\r\n");
				usleep(10000);
				uart_ack();
				//uart_index_ack(VREFH_VREFL_REQUEST);
			}
			else	
			{
				if(cprf)	printf("VREFH_VREFL_WRITE_NG\r\n");
				error_data.i2c_communication = ERROR_NG;				
				usleep(10000);
				uart_nack();	//230929 Modify								
			}
			//goto DEFLAT;
		}	
		//else if((com_buffer[0] == PATTERN_R_G_B_REQUEST) && (com_cnt == PATTERN_R_G_B_REQUEST_LEN) && (pg_on_flag == PG_ON))
		else if((com_buffer[0] == PATTERN_R_G_B_REQUEST) && (com_cnt == PATTERN_R_G_B_REQUEST_LEN))
		{
			rgb_voltage_request();
		}			
		else if((com_buffer[0] == ZONE_SELECT) && (com_cnt == ZONE_SELECT_REQUEST_LEN))
		{
			zone_select = com_buffer[1]; 	
			if(cprf)	printf("ZONE = %x\r\n", zone_select);	
			uart_ack();
			//uart_index_ack(ZONE_SELECT);	
		}
		else if((com_buffer[0] == PATTERN_SELECT) && (com_cnt == PATTERN_SELECT_LEN))
		{
			unsigned char pattern_select_index = 0;
			//cur_sensing_reset_flag = 1;	       //231013 Modify
			pg_on_flag = PG_ON;
			pat_index = com_buffer[1];
			if(zone_select == 0x01)	sprintf(pat_string_index, "T%d",pat_index);
			else if(zone_select == 0x02)	sprintf(pat_string_index, "H%d",pat_index);
			else if(zone_select == 0x03)	sprintf(pat_string_index, "M%d",pat_index);
			else sprintf(pat_string_index, "%d",pat_index);
			if(cprf)	printf("ZONE = [%d]TURN_ON_ok = [%s]\r\n", zone_select, pat_string_index);
			if(system_load_flag==0)
			{
				memset(&error_data, 0, sizeof(error_data));
				recipe_system_load();
				system_load_flag = 1;
				pattern_select_index = 1;
			}
			if(pg_on_flag != PG_OFF)	recipe_funtion_load(pat_string_index);	
			else	system_load_flag = 0;
			if(pg_on_flag == PG_OFF)	system_load_flag = 0; 

			if(pattern_select_index==1)
			{
				avdd_vol_2byte = 0;
				elvss_vol_2byte = 0;
				ADC_SELECT_DATA_READ_AVG(SEN_AVDD);
				avdd_vol_2byte = adc_sensing_value[SEN_AVDD];	
				if(vprf)	printf("AVDD = %d\r\n", avdd_vol_2byte);
				ADC_SELECT_DATA_READ_AVG(SEN_ELVSS);
				elvss_vol_2byte = adc_sensing_value[SEN_ELVSS];	
				if(vprf) 	printf("ELVSS = %d\r\n", elvss_vol_2byte);	
				com_task_ack(AVDD_ELVSS_VOLTAGE_REQUEST); 
			}				
			uart_ack(); 		
			//uart_index_ack(PATTERN_SELECT);		
			//goto ACK;				
		}
		else if((com_buffer[0] == REGISTER_OFFSET_WRITE) && (com_cnt == REGISTER_OFFSET_WRITE_LEN))
		{
			unsigned short data_2byte = 0;
			double data_8byte = 0;
			data_2byte = (com_buffer[3]<<8) | (com_buffer[2]);
			if(com_buffer[1] == 0)
			{
				if(data_2byte & 0x8000)
				{
					data_2byte = ((~data_2byte & 0x7fff)+1); 
					data_8byte = (double)-data_2byte;
				}
				else 
				{
					data_8byte = (double)data_2byte;
				}
				register_offset.register_r = data_8byte/10000;
			}
			else if(com_buffer[1] == 1)
			{
				if(data_2byte & 0x8000)
				{
					data_2byte = ((~data_2byte & 0x7fff)+1); 
					data_8byte = (double)-data_2byte;
				}
				else 
				{
					data_8byte = (double)data_2byte;
				}
				register_offset.register_l = data_8byte/10000;				
			}
			printf("register_offset.register_r = %f / register_offset.register_l = %f\r\n", register_offset.register_r, register_offset.register_l);	
			//goto DEFLAT;
		}
		else if((com_buffer[0] == REGISTER_OFFSET_SAVE) && (com_cnt == REGISTER_OFFSET_SAVE_LEN))
		{
			FILE *adc_file;

			adc_file = fopen(REGISTER_OFFSET_FILE_PATH, "w+b");
			if(adc_file != NULL)
			{
				fwrite(&register_offset, sizeof(register_offset), 1, adc_file);
				fclose(adc_file);
				system("sync");	
				printf("register_offset_data_save_OK\r\n");	
				//goto DEFLAT;			
			}
		}							
		else if(com_buffer[0] == I2C_WRITE_REQUEST)
		{
			unsigned char slave_address = i2c_wr_addr>>1;
			int reg_address = 0;
			int write_byte = 0;
			int i = 0;
			char write_buffer[10] = {0};  			
			write_byte = com_cnt - 3;
			reg_address = (com_buffer[1] << 8) | com_buffer[2];

			if(reg_address >= 0x1000)	slave_address = i2c_test_wr_addr>>1;

			for(i = 0 ; i < write_byte ; i++)
			{
				write_buffer[i] = com_buffer[3+i];	
			}
			i2c_write(slave_address,reg_address,write_buffer,write_byte);
			usleep(USER_DELAY);  				
		}
		else if((com_buffer[0] == I2C_READ_REQUEST) && (com_cnt == I2C_READ_REQUEST_LEN))
		{
			unsigned char slave_address = i2c_rd_addr>>1;
			int read_byte = 0;
			int reg_address = 0;
			int i = 0;
			char read_buffer[20] = {0};
			char str[20]={0};
			unsigned short checksum=0;
			unsigned short tx_len = 0;		

			read_byte = com_buffer[3];
			reg_address = (com_buffer[1] << 8) | com_buffer[2];

			if(reg_address >= 0x1000)	slave_address = i2c_test_rd_addr>>1;

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
			if(read_byte <8)
			{
				if(read_byte == 1)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;					
				}
				else if(read_byte == 2)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;						
				}
				else if(read_byte == 3)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
				}
				else if(read_byte == 4)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
				}
				else if(read_byte == 5)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
				}
				else if(read_byte == 6)
				{
					str[tx_len++] = 0x00;
					str[tx_len++] = 0x00;
				}
				else if(read_byte == 7)
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
				printf("read_buffer[0] = %x\r\n", read_buffer[0]); 
				printf("read_buffer[1] = %x\r\n", read_buffer[1]);
				printf("read_buffer[2] = %x\r\n", read_buffer[2]); 
				printf("read_buffer[3] = %x\r\n", read_buffer[3]); 
				printf("read_buffer[4] = %x\r\n", read_buffer[4]); 
				printf("read_buffer[5] = %x\r\n", read_buffer[5]);
				printf("read_buffer[6] = %x\r\n", read_buffer[6]); 
				printf("read_buffer[7] = %x\r\n", read_buffer[7]);  				        
			} 
			for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
			str[tx_len++] = checksum;
			str[tx_len++] = 0x03;
			sdcd_serial_write(tx_len, str);
			memset(&str, 0, sizeof(str));
			usleep(USER_DELAY); 
			 						
		}
		else if((com_buffer[0] == SHORT_CHECK_REQUEST) && (com_cnt == SHORT_CHECK_REQUEST_LEN))
		{
			short vol = 0;			
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_4000SPS);
			vol = (com_buffer[3] << 8) | com_buffer[2];
			bist_test(com_buffer[1], vol);
			if(aprf) ADC_DATA_PRINT();
			com_task_ack(SHORT_CHECK_REQUEST);		
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);
			//goto DEFLAT;				
		}	
		else if((com_buffer[0] == AVDD_ELVSS_VOLTAGE_REQUEST) && (com_cnt == AVDD_ELVSS_VOLTAGE_REQUEST_LEN))	//231013 Modify
		{
			avdd_vol_2byte = 0;
			elvss_vol_2byte = 0;
			ADC_SELECT_DATA_READ_AVG(SEN_AVDD);
			avdd_vol_2byte = adc_sensing_value[SEN_AVDD];	
			if(vprf)	printf("AVDD = %d\r\n", avdd_vol_2byte);
			ADC_SELECT_DATA_READ_AVG(SEN_ELVSS);
			elvss_vol_2byte = adc_sensing_value[SEN_ELVSS];	
			if(vprf) 	printf("ELVSS = %d\r\n", elvss_vol_2byte);	
			com_task_ack(AVDD_ELVSS_VOLTAGE_REQUEST); 
		}		
		else if((com_buffer[0] == AVDD_ELVSS_CURRENT_REQUEST) && (com_cnt == AVDD_ELVSS_CURRENT_REQUEST_LEN))	//231013 Modify
		{
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
			ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
			elvss_2byte = fabs(elvss_cur_sensing_data*10);            
			ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
			avdd_2byte = fabs(avdd_cur_sensing_data*10);                                    
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);

			if(vprf)	printf("AVDD_CURRENT = %d\r\n", avdd_2byte);	
			if(vprf) 	printf("ELVSS_CURRENT = %d\r\n", elvss_2byte);	
			com_sensing_task_ack(AVDD_ELVSS_CURRENT_REQUEST,(unsigned char*)&avdd_2byte,(unsigned char*)&elvss_2byte);  
		}
		else if((com_buffer[0] == VOLTAGE_VARIABLE_REQUEST) && (com_cnt == VOLTAGE_VARIABLE_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			if(com_buffer[2] == 0x00)	signal_group.signal_config.dc_voltage[com_buffer[1]] += 100;
			else	signal_group.signal_config.dc_voltage[com_buffer[1]] -= 100; 	

			Power_Supply_Voltage_load();
			SET_DAC_OUTPUT_VALUE(com_buffer[1]);			
		}
		else if((com_buffer[0] == BIST_PATTERN_SHTTING_REQUEST) && (com_cnt == BIST_PATTERN_SHTTING_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			//cur_sensing_reset_flag = 1;	//231013 Modify						
			if(com_buffer[1] == 0x00)	i2c_com_bist_pattern("W",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x01)	i2c_com_bist_pattern("R",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x02)	i2c_com_bist_pattern("G",com_buffer[2],FULL_COLOR_MODE1);  
			else if(com_buffer[1] == 0x03)	i2c_com_bist_pattern("B",com_buffer[2],FULL_COLOR_MODE1);  
		}	
		else if((com_buffer[0] == ELVSS_4BYTE_CURRENT_REQUEST) && (com_cnt == ELVSS_4BYTE_CURRENT_REQUEST_LEN) && (pg_on_flag == PG_ON))	//231013 Modify
		{
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
			ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
			elvss_4byte = fabs(elvss_cur_sensing_data*1000);                 
			ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
			avdd_4byte = fabs(avdd_cur_sensing_data*1000);                
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);

			if(vprf)	printf("AVDD_4BYTE_CURRENT = %d\r\n", avdd_4byte);	
			if(vprf) 	printf("ELVSS_4BYTE_CURRENT = %d\r\n", elvss_4byte);  

			com_sensing_task_ack(ELVSS_4BYTE_CURRENT_REQUEST,(unsigned char*)&avdd_4byte,(unsigned char*)&elvss_4byte);
		}
		else if((com_buffer[0] == SENSING_DATA_CUR_4BYTE_REQUEST) && (com_cnt == SENSING_DATA_CUR_4BYTE_REQUEST_LEN) && (pg_on_flag == PG_ON))	//231013 Modify
		{
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
			ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
			elvss_4byte = fabs(elvss_cur_sensing_data*1000);            
			ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
			avdd_4byte = fabs(avdd_cur_sensing_data*1000);                    
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);

			ADC_AUTO_DATA_READ();
			cur_sensing_value[0] = avdd_4byte;
			cur_sensing_value[1] = elvss_4byte;
			total_sen_cur_4byte_flag = 1;
			if(aprf) ADC_DATA_PRINT();
			total_sen_cur_4byte_flag = 0;
			com_task_ack(SENSING_DATA_CUR_4BYTE_REQUEST); 
		}
		else if((com_buffer[0] == AVDD_ELVSS_4BYTE_CURRENT_REQUEST) && (com_cnt == AVDD_ELVSS_4BYTE_CURRENT_REQUEST_LEN) && (pg_on_flag == PG_ON))	//231013 Modify
		{
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_16_6SPS);
			ADC_SELECT_DATA_READ_AVG(SEN_ELISS);
			elvss_4byte = fabs(elvss_cur_sensing_data*1000);                 
			ADC_SELECT_DATA_READ_AVG(SEN_ELIDD);
			avdd_4byte = fabs(avdd_cur_sensing_data*1000);                
			ads124s08_short_check_set(DATARATE_ADDRESS,DATARATE_SINC3_200SPS);

			if(vprf)	printf("AVDD_4BYTE_CURRENT = %d\r\n", avdd_4byte);	
			if(vprf) 	printf("ELVSS_4BYTE_CURRENT = %d\r\n", elvss_4byte);	

			com_sensing_task_ack(AVDD_ELVSS_4BYTE_CURRENT_REQUEST,(unsigned char*)&avdd_4byte,(unsigned char*)&elvss_4byte);
		}
		else if((com_buffer[0] == TEMPERATURE_SENSING_REQUEST) && (com_cnt == TEMPERATURE_SENSING_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			unsigned char slave_address = i2c_rd_addr>>1;
			int read_byte = 0;
			int reg_address = 0;
			int i = 0;
			char read_buffer[10] = {0};
			float temp[4] = {0};
			char str[20]={0};
			unsigned short checksum=0;
			unsigned short tx_len = 0;
			
			read_byte = 4;
			reg_address = LT_BPRD_1ST_ADDR;

			i2c_read(slave_address,reg_address,read_buffer,read_byte);

			temp[0] = (read_buffer[0]-15)*0.75-40;
			temp[1] = (read_buffer[1]-15)*0.75-40;
			temp[2] = (read_buffer[2]-15)*0.75-40;
			temp[3] = (read_buffer[3]-15)*0.75-40;

			str[tx_len++] = 0x02;
			str[tx_len++] = TEMPERATURE_SENSING_REQUEST;
			str[tx_len++] = 0x80;	
			str[tx_len++] = TEMPERATURE_SENSING_REQUEST_ACK_LEN;
			str[tx_len++] = (short)(temp[0]*100) >>8;
			str[tx_len++] = (short)(temp[0]*100)&0x00ff;
			str[tx_len++] = (short)(temp[1]*100) >>8;
			str[tx_len++] = (short)(temp[1]*100)&0x00ff;
			str[tx_len++] = (short)(temp[2]*100) >>8;
			str[tx_len++] = (short)(temp[2]*100)&0x00ff;	
			str[tx_len++] = (short)(temp[3]*100) >>8;
			str[tx_len++] = (short)(temp[3]*100)&0x00ff;				

			if(cprf)
			{
				printf("SLAVE_ADDRESS = %x\r\n", slave_address);
				printf("REG_ADDRESS = %x\r\n", reg_address);
				printf("read_byte = %x\r\n", read_byte);
				printf("address = %x\r\n", com_buffer[1]);
				printf("address = %x\r\n", com_buffer[2]); 
				printf("read_buffer[0] = %x\r\n", read_buffer[0]); 
				printf("read_buffer[1] = %x\r\n", read_buffer[1]);
				printf("read_buffer[2] = %x\r\n", read_buffer[2]); 
				printf("read_buffer[3] = %x\r\n", read_buffer[3]);  
				printf("temp[0] = %f\r\n", temp[0]); 
				printf("temp[1] = %f\r\n", temp[1]); 
				printf("temp[2] = %f\r\n", temp[2]); 
				printf("temp[3] = %f\r\n", temp[3]); 																												       
			} 
			for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
			str[tx_len++] = checksum;
			str[tx_len++] = 0x03;
			sdcd_serial_write(tx_len, str);
			memset(&str, 0, sizeof(str));
			//goto DEFLAT;
		}
		else if((com_buffer[0] == LDO_REGISTER_RAED_REQUEST) && (com_cnt == LDO_REGISTER_RAED_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			unsigned char slave_address = i2c_rd_addr>>1;
			int read_byte = 0;
			int reg_address = 0;
			int i = 0;
			char read_buffer[10] = {0};
			char str[20]={0};
			unsigned short checksum=0;
			unsigned short tx_len = 0;
			
			read_byte = 1;
			
			if(com_buffer[1] == LDO_VREG)	reg_address = VREFH_ADDR;	
			else if(com_buffer[1] == LDO_VREF)	reg_address = VREFL_ADDR;	
			else if(com_buffer[1] == LDO_VGH)	reg_address = VGH_ADDR;	
			else if(com_buffer[1] == LDO_VGL)	reg_address = VGL_ADDR;	
			else if(com_buffer[1] == LDO_VINT)	reg_address = VINT_ADDR;	
			
			i2c_read(slave_address,reg_address,read_buffer,read_byte);

			str[tx_len++] = 0x02;
			str[tx_len++] = LDO_REGISTER_RAED_REQUEST;
			str[tx_len++] = 0x80;	
			str[tx_len++] = LDO_REGISTER_RAED_REQUEST_ACK_LEN;
			str[tx_len++] = read_buffer[0];
			str[tx_len++] = com_buffer[1];
				
			if(cprf)
			{
				printf("SLAVE_ADDRESS = %x\r\n", slave_address);
				printf("REG_ADDRESS = %x\r\n", reg_address);
				printf("read_byte = %x\r\n", read_byte);
				printf("address = %x\r\n", com_buffer[1]);
				printf("address = %x\r\n", com_buffer[2]); 
				printf("read_buffer[0] = %x\r\n", read_buffer[0]); 																												       
			} 
			for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
			str[tx_len++] = checksum;
			str[tx_len++] = 0x03;
			sdcd_serial_write(tx_len, str);
			memset(&str, 0, sizeof(str));
			//goto DEFLAT;
		}
		else if((com_buffer[0] == LDO_REGISTER_WRITE_REQUEST) && (com_cnt == LDO_REGISTER_WRITE_REQUEST_LEN) && (pg_on_flag == PG_ON))
		{
			unsigned char slave_address = i2c_wr_addr>>1;
			int reg_address = 0;
			int write_byte = 0;
			int i = 0;
			char write_buffer[10] = {0};  			
			write_byte = 1;

			if(com_buffer[1] == LDO_VREG)	reg_address = VREFH_ADDR;	
			else if(com_buffer[1] == LDO_VREF)	reg_address = VREFL_ADDR;	
			else if(com_buffer[1] == LDO_VGH)	reg_address = VGH_ADDR;	
			else if(com_buffer[1] == LDO_VGL)	reg_address = VGL_ADDR;	
			else if(com_buffer[1] == LDO_VINT)	reg_address = VINT_ADDR;	

			write_buffer[0] = com_buffer[2];
			i2c_write(slave_address,reg_address,write_buffer,write_byte);
			usleep(USER_DELAY); 
			
			uart_ack();
			//goto ACK;
		}
		else if((com_buffer[0] == AVDD_ELVSS_CUR_OFFSET_REQUEST) && (com_cnt == AVDD_ELVSS_CUR_OFFSET_REQUEST_LEN))
		{
			unsigned short data_2byte = 0;
			double data_8byte = 0;
			data_2byte = (com_buffer[3]<<8) | (com_buffer[2]);
			if(com_buffer[1] == AVDD_POSITIVE)
			{
				if(data_2byte & 0x8000)
				{
					data_2byte = ((~data_2byte & 0x7fff)+1); 
					data_8byte = (double)-data_2byte;
				}
				else 
				{
					data_8byte = (double)data_2byte;
				}
				avdd_cur_cal.cur_1a_user_offset = data_8byte/1000;
				printf("AVDD_POSITIVE_OFFSET = %f\r\n",avdd_cur_cal.cur_1a_user_offset);
			}
			else if(com_buffer[1] == AVDD_NEGATIVE)
			{
				if(data_2byte & 0x8000)
				{
					data_2byte = ((~data_2byte & 0x7fff)+1); 
					data_8byte = (double)-data_2byte;
				}
				else 
				{
					data_8byte = (double)data_2byte;
				}
				avdd_cur_cal.n_cur_1a_user_offset = data_8byte/1000;	
				printf("AVDD_NEGATIVE_OFFSET = %f\r\n",avdd_cur_cal.n_cur_1a_user_offset);			
			}
			else if(com_buffer[1] == ELVSS_POSITIVE)
			{
				if(data_2byte & 0x8000)
				{
					data_2byte = ((~data_2byte & 0x7fff)+1); 
					data_8byte = (double)-data_2byte;
				}
				else 
				{
					data_8byte = (double)data_2byte;
				}
				elvss_cur_cal.cur_1a_user_offset = data_8byte/1000;	
				printf("ELVSS_POSITIVE_OFFSET = %f\r\n",elvss_cur_cal.cur_1a_user_offset);				
			}
			else if(com_buffer[1] == ELVSS_NEGATIVE)
			{
				if(data_2byte & 0x8000)
				{
					data_2byte = ((~data_2byte & 0x7fff)+1); 
					data_8byte = (double)-data_2byte;
				}
				else 
				{
					data_8byte = (double)data_2byte;
				}
				elvss_cur_cal.n_cur_1a_user_offset = data_8byte/1000;
				printf("ELVSS_NEGATIVE_OFFSET = %f\r\n",elvss_cur_cal.n_cur_1a_user_offset);				
			}							
			//goto DEFLAT;
		}
		else if((com_buffer[0] == AVDD_ELVSS_CUR_OFFSET_SAVE_REQUEST) && (com_cnt == AVDD_ELVSS_CUR_OFFSET_SAVE_REQUEST_LEN))
		{
			FILE *adc_file;

			if(com_buffer[1] == AVDD_OFFSET_SAVE)
			{
				adc_file = fopen(AVDD_CUR_CAL_FILE_PATH, "w+b");
				if(adc_file !=NULL)
				{
					fwrite(&avdd_cur_cal, sizeof(avdd_cur_cal), 1, adc_file);
					fclose(adc_file);
					system("sync");	
					printf("cur_cal_data_save_OK\r\n");
				}		
			}
			else
			{
				adc_file = fopen(ELVSS_CUR_CAL_FILE_PATH, "w+b");
				if(adc_file !=NULL)
				{				
					fwrite(&elvss_cur_cal, sizeof(elvss_cur_cal), 1, adc_file);
					fclose(adc_file);
					system("sync");	
					printf("elvss_cur_cal_data_save_OK\r\n");
				}
			}			
		}
		else if((com_buffer[0] == I2C_TEST_REQUEST) && (com_cnt == I2C_TEST_REQUEST_LEN))
		{
			unsigned char slave_address = i2c_wr_addr>>1;
			int reg_address = 0;
			int write_byte = 0;
			int read_byte = 0;
			int i = 0;
			char write_buffer[10] = {0};  
			char read_buffer[10] = {0};	
			char read_buffer_pre = 0;	
			char result = 0;	
			char str[20]={0};
			unsigned short checksum=0;
			unsigned short tx_len = 0;					
			write_byte = 1;
			read_byte = 1;

			if(com_buffer[1] == EP971_FLASH)
			{
				reg_address = 0x1000;	
			}
			else	
			{
				reg_address = 	VREFH_ADDR;
			}

			if(reg_address >= 0x1000)	slave_address = i2c_test_rd_addr>>1;

			i2c_read(slave_address,reg_address,read_buffer,read_byte);
			read_buffer_pre = read_buffer[0];
			if(cprf)	printf("Existing value before I2C test = %x\r\n", read_buffer_pre);
			write_buffer[0] = 0x50;
			usleep(USER_DELAY); 
			i2c_write(slave_address,reg_address,write_buffer,write_byte);
			usleep(USER_DELAY); 
			i2c_read(slave_address,reg_address,read_buffer,read_byte);
			if(read_buffer[0] == 0x50)	result = I2C_TEST_OK;
			else		result = I2C_TEST_NG; 
			write_buffer[0] = read_buffer_pre;
			usleep(USER_DELAY); 
			i2c_write(slave_address,reg_address,write_buffer,write_byte);	

			str[tx_len++] = 0x02;
			str[tx_len++] = I2C_TEST_REQUEST;
			str[tx_len++] = 0x80;	
			str[tx_len++] = 0x02;
			str[tx_len++] = com_buffer[1];
			str[tx_len++] = result;				
			for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
			str[tx_len++] = checksum;
			str[tx_len++] = 0x03;
			sdcd_serial_write(tx_len, str);
			memset(&str, 0, sizeof(str));					
			//goto DEFLAT;
		}
		else if((com_buffer[0] == BIST_PATTERN_DATA_READ_REQUEST) && (com_cnt == BIST_PATTERN_DATA_READ_REQUEST_LEN))
		{
			bist_pattern_read_data_request();
			//goto DEFLAT;
		}	
		else if((com_buffer[0] == ERROR_DATA_REQUEST) && (com_cnt == ERROR_DATA_REQUEST_LEN))
		{
			com_task_ack(ERROR_DATA_REQUEST);
			//goto DEFLAT;
		}
		else if((com_buffer[0] == ERROR_DATA_RESET_REQUEST) && (com_cnt == ERROR_DATA_RESET_REQUEST_LEN))
		{
			memset(&error_data, 0, sizeof(error_data));	
			uart_ack();
		}
		else if((com_buffer[0] == CURRENT_MODEL_INDEX_REQUEST) && (com_cnt == CURRENT_MODEL_INDEX_REQUEST_LEN))
		{
			com_task_ack(CURRENT_MODEL_INDEX_REQUEST);	
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
			uart_ack();
			//goto ACK;	
		}
		else if((com_buffer[0] == FW_FPGA_DOWNLOAD_START) && (com_cnt == FW_FPGA_DOWNLOAD_LEN) && (pg_on_flag == PG_OFF))
		{
			unsigned char fw_flag= 0;
            if(fw_fpga.download_mode != 1)
			{
                if(cprf)	printf("CH[%02d] No Download Mode\r\n", fw_fpga.download_ch);
				uart_nack();
				fw_flag = 1;
                //goto NACK;
			}
            if(recipe_download_cnt != (fw_fpga.download_packetnumber & 0xFF))
			{
                if(cprf)	printf("CH[%02d] Packet Numer Error (Recv 0x%02x, Current 0x%02x)\r\n", fw_fpga.download_ch, recipe_download_cnt, (fw_fpga.download_packetnumber&0xFF));
				if(recipe_download_cnt < fw_fpga.download_packetnumber)	uart_ack();	//231013 Modify
				else uart_nack();
				fw_flag = 1;
                //goto NACK;
            }
			if(!fw_flag)
			{
				FILE *model_file = NULL;
				if(fw_fpga.download_packetnumber == 0)
				{
					if(access("/f0/fw_fpga", F_OK) != 0) system("mkdir /f0/fw_fpga");
					model_file = fopen(FW_FPGA_FILE_PATH, "w+b");
				}
				else model_file = fopen(FW_FPGA_FILE_PATH, "ab");

				if(model_file != NULL)
				{
					fwrite(&com_buffer[1],(fw_fpga.download_length >= 1024) ? 1024 : fw_fpga.download_length,1,model_file);
					fclose(model_file);
					if(cprf)	printf("fw_fpga File write, Packet Num = %d\r\n", fw_fpga.download_packetnumber);	
					fw_fpga.download_packetnumber++;
					fw_fpga.download_length -= (fw_fpga.download_length >= 1024) ? 1024 : fw_fpga.download_length;
					usleep(100000);   //Stabilize until fwrite			//230929 Modify	
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
			}			
		}			
		else if((com_buffer[0] == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_START_LEN) && (pg_on_flag == PG_OFF))
		{			
            recipe.download_ch = 0;
            recipe.download_packetnumber = 0;
            recipe.download_mode = 1;
            recipe.download_length = (com_buffer[1] << 24)|(com_buffer[2] << 16)|(com_buffer[3] << 8)|(com_buffer[4] << 0);
            if(cprf)	printf("CH[%02d] Recipe Download Length = %d\r\n", recipe.download_ch, recipe.download_length);	
			uart_ack();
			//goto ACK;	
		}
		else if((com_buffer[0] == RECIPE_DOWNLOAD_START) && (com_cnt == RECIPE_DOWNLOAD_LEN) && (pg_on_flag == PG_OFF))
		{
			unsigned char recipe_flag = 0;
			system_error.recipe_copy = ERROR_OK;
			system_error.model_download_file_open =	ERROR_OK; 
            if(recipe.download_mode != 1)
			{
                if(cprf)	printf("CH[%02d] No Download Mode\r\n", recipe.download_ch);
				uart_nack();
				recipe_flag = 1;
                //goto NACK;
			}
            if(recipe_download_cnt != (recipe.download_packetnumber & 0xFF))
			{
                if(cprf)	printf("CH[%02d] Packet Numer Error (Recv 0x%02x, Current 0x%02x)\r\n", recipe.download_ch, recipe_download_cnt, (recipe.download_packetnumber&0xFF));
				if(recipe_download_cnt < recipe.download_packetnumber)	uart_ack();	//231101 Modify
				else	uart_nack();	//231101 Modify
				recipe_flag = 1;
                //goto NACK;
            }
			if(!recipe_flag)
			{
				FILE *model_file = NULL;			
				if(recipe.download_packetnumber == 0)
				{
					if(access("/f0/recipe", F_OK) != 0) system("mkdir /f0/recipe");
					model_file = fopen(TEMPORARY_RECIPE_FILE_PATH, "w");
				}
				else 
				{
					//model_file = fopen(TEMPORARY_RECIPE_FILE_PATH, "ab");
					model_file = fopen(TEMPORARY_RECIPE_FILE_PATH, "a");
				}
				if(model_file != NULL)
				{
					fwrite(&com_buffer[1],(recipe.download_length >= 1024) ? 1024 : recipe.download_length,1,model_file);
					fclose(model_file);
					if(cprf)	printf("Recipe File write, Packet Num = %d\r\n", recipe.download_packetnumber);	
					recipe.download_packetnumber++;
					recipe.download_length -= (recipe.download_length >= 1024) ? 1024 : recipe.download_length;
					usleep(100000);   //Stabilize until fwrite			//230929 Modify	
					if(recipe.download_length == 0)
					{
						recipe_download_task();			//230929 Modify	
					}					
					if((system_error.recipe_copy != ERROR_NG) && (system_error.model_download_file_open != ERROR_NG))	uart_ack();	//230929 Modify	
				}
				else	//230929 Modify	 
				{
					printf("RECIPE_FILE OPEN_ERROR\r\n");
					system_error.model_download_file_open = ERROR_NG;
				}
			}		
		}
		else if((com_buffer[0] == OUTPUT_CHECK_REQUEST) && (com_cnt == OUTPUT_CHECK_REQUEST_LEN))	//231206 Modify
		{
			pg_check_task();		
		}	
		else if((com_buffer[0] == KEITHLEY_CONNECTION_CHECK_REQUEST) && (com_cnt == KEITHLEY_CONNECTION_CHECK_REQUEST_LEN))	//231206 Modify
		{
			unsigned char result = 0; 
			result = vol_cur_select_task(SELECT_VOL);
			ext_com_task_ack(OUTPUT_CHECK_REQUEST,OUTPUT_CHECK_REQUEST_LEN,&result);	
		}
		else if((com_buffer[0] == DAC_DATA_INIT_REQUEST) && (com_cnt == DAC_DATA_INIT_REQUEST_LEN))	//231206 Modify
		{
			memset(&dac_cal, 0, sizeof(dac_cal));	
			serial_dev_ack_nack_task(DAC_DATA_INIT_REQUEST,DAC_DATA_INIT_REQUEST_LEN,AUTO_TASK_OK);	
		}	
		else if((com_buffer[0] == DAC_AUTO_CAL_REQUEST) && (com_cnt == DAC_AUTO_CAL_REQUEST_LEN))	//231206 Modify
		{
			unsigned char result = 0;
			//usleep(100000);
			result = dac_auto_cal(com_buffer[1]);
			if(pprf)	printf("DAC_AUTO_CAL_RESULT = %d\r\n",result);	//OK : 0x00, NG : 0x50(Max count excess), 0xff(Keithley com error), etc(ch error chnum+1)	
			power_off(); 	
			cal_end_task();
			usleep(100000);
			if(result != AUTO_TASK_OK)	serial_dev_ack_nack_task(DAC_AUTO_CAL_REQUEST,DAC_AUTO_CAL_REQUEST_LEN,AUTO_TASK_NG);	
			else	serial_dev_ack_nack_task(DAC_AUTO_CAL_REQUEST,DAC_AUTO_CAL_REQUEST_LEN,AUTO_TASK_OK);	 					
		}	
		else if((com_buffer[0] == DAC_AUTO_OFFSET_REQUEST) && (com_cnt == DAC_AUTO_OFFSET_REQUEST_LEN))	//231206 Modify
		{
			unsigned char result = 0;	
			dac_cal[com_buffer[1]].dac_10to25_offset = 0;	
			dac_cal[com_buffer[1]].dac_0to10_offset = 0; 
			dac_cal[com_buffer[1]].dac_0ton10_offset = 0; 
			dac_cal[com_buffer[1]].dac_n10to_n25_offset = 0; 			
			result = dac_auto_offset_cal(com_buffer[1]);
			if(pprf)	printf("DAC_AUTO_OFFSET_RESULT = %d\r\n",result);	//OK : 0x00, NG : 0xff(Keithley com error)
			power_off(); 	
			cal_end_task();	
			usleep(100000);	
			if(result != AUTO_TASK_OK)	serial_dev_ack_nack_task(DAC_AUTO_OFFSET_REQUEST,DAC_AUTO_OFFSET_REQUEST_LEN,AUTO_TASK_NG);	
			else	serial_dev_ack_nack_task(DAC_AUTO_OFFSET_REQUEST,DAC_AUTO_OFFSET_REQUEST_LEN,AUTO_TASK_OK);	 				
		}	
		else if((com_buffer[0] == DAC_CHECK_REQUEST) && (com_cnt == DAC_CHECK_REQUEST_LEN))	//231206 Modify
		{	
			unsigned char result = 0;			
			result = dac_vol_check(com_buffer[1]);
			if(pprf)	printf("DAC_CHECK_RESULT = %d\r\n",result);		//OK : 0x00, NG : 0x01(11~14mV), 0x02(15mV~), 0xff(Keithley com error)
			power_off(); 
			cal_end_task();	
			usleep(100000);		
			serial_dev_ack_nack_task(DAC_CHECK_REQUEST,DAC_CHECK_REQUEST_LEN,result);				
		}
		else if((com_buffer[0] == DAC_DATA_SAVE_REQUEST) && (com_cnt == DAC_DATA_SAVE_REQUEST_LEN))	//231206 Modify
		{
			FILE *dac_file;
			dac_file = fopen(DAC_CAL_FILE_PATH, "w+b");
			if(!dac_file)
			{
				fwrite(&dac_cal, sizeof(dac_cal), 1, dac_file);
				fclose(dac_file);
				system("sync");	
				if(pprf)	printf("dac_data_save_OK\r\n");	
				serial_dev_ack_nack_task(DAC_DATA_INIT_REQUEST,DAC_DATA_INIT_REQUEST_LEN,AUTO_TASK_OK);		
			}
			else 
			{
				if(pprf)	printf("dac_data_save_Fail\r\n");	
				serial_dev_ack_nack_task(DAC_DATA_INIT_REQUEST,DAC_DATA_INIT_REQUEST_LEN,AUTO_TASK_NG);					
			}					
		}
		else if((com_buffer[0] == ADC_DATA_INIT_REQUEST) && (com_cnt == ADC_DATA_INIT_REQUEST_LEN))	//231206 Modify
		{
			memset(&ads124_cal0, 0, sizeof(ads124_cal0));
			memset(&ads124_cal1, 0, sizeof(ads124_cal1));
			memset(&ads124_cal2, 0, sizeof(ads124_cal2));
			memset(&ads124_cal3, 0, sizeof(ads124_cal3));
			memset(&ads124_cal4, 0, sizeof(ads124_cal4));	
			memset(&vol_offset, 0, sizeof(vol_offset)); 															
			if(pprf)	printf("ads124 cal & offset data init\r\n");
			serial_dev_ack_nack_task(ADC_DATA_INIT_REQUEST,ADC_DATA_INIT_REQUEST_LEN,AUTO_TASK_OK);	
		}
		else if((com_buffer[0] == ADC_AUTO_CAL_REQUEST) && (com_cnt == ADC_AUTO_CAL_REQUEST_LEN))	//231206 Modify
		{
			unsigned char result = 0;	
			result = ex_auto_cal_task();
			power_off(); 	
			cal_end_task();
			usleep(100000);
			if(result != AUTO_TASK_OK)	serial_dev_ack_nack_task(DAC_AUTO_OFFSET_REQUEST,DAC_AUTO_OFFSET_REQUEST_LEN,AUTO_TASK_NG);	
			else	serial_dev_ack_nack_task(DAC_AUTO_OFFSET_REQUEST,DAC_AUTO_OFFSET_REQUEST_LEN,AUTO_TASK_OK);	 						
		}
		else if((com_buffer[0] == ADC_AUTO_OFFSET_REQUEST) && (com_cnt == ADC_AUTO_OFFSET_REQUEST_LEN))	//231206 Modify
		{
			int i = 0;
			unsigned char result = 0;
			usleep(100000);
			for(i = 0; i < EX_CAL_LM_SPARE2+1 ; i++)
			{
				result = adc_offset_task(i);
				if(pprf)	printf("ADC_AUTO_OFFSET_RESULT = %x\r\n",result);
				if(result==DAC_AUTOCHECK_CANCEL)	break;
			}
			power_off(); 	
			cal_end_task();	
			usleep(100000);	
			if(result != AUTO_TASK_OK)	serial_dev_ack_nack_task(ADC_AUTO_OFFSET_REQUEST,ADC_AUTO_OFFSET_REQUEST_LEN,AUTO_TASK_NG);	
			else	serial_dev_ack_nack_task(ADC_AUTO_OFFSET_REQUEST,ADC_AUTO_OFFSET_REQUEST_LEN,AUTO_TASK_OK);	 					
		}
		else if((com_buffer[0] == ADC_DATA_SAVE_REQUEST) && (com_cnt == ADC_DATA_SAVE_REQUEST_LEN))	//231206 Modify
		{
			adc_cal_offset_data_save_task();					
		}
		else if((com_buffer[0] == LDO_ON_OFF_REQUEST) && (com_cnt == LDO_ON_OFF_REQUEST_LEN))	//231206 Modify
		{
			if(com_buffer[0] == LDO_CONNECT_ON)	ldo_all_on_off_task(EX_LDO_ALL_ON); 	
			else	ldo_all_on_off_task(EX_LDO_ALL_OFF); 	
			usleep(100000);
			serial_dev_ack_nack_task(LDO_ON_OFF_REQUEST,LDO_ON_OFF_REQUEST_LEN,AUTO_TASK_OK);	 	
		}
		else if((com_buffer[0] == CUR_DATA_INIT_REQUEST) && (com_cnt == CUR_DATA_INIT_REQUEST_LEN))	//231206 Modify
		{
			memset(&avdd_cur_cal, 0, sizeof(avdd_cur_cal));
			memset(&elvss_cur_cal, 0, sizeof(elvss_cur_cal));
			memset(&n_avdd_cur_cal, 0, sizeof(n_avdd_cur_cal));
			memset(&n_elvss_cur_cal, 0, sizeof(n_elvss_cur_cal));
			memset(&elvss_cur_offset_cal, 0, sizeof(elvss_cur_offset_cal));
			memset(&avdd_cur_offset_cal, 0, sizeof(avdd_cur_offset_cal));																
			if(pprf)	printf("cur cal & offset data init\r\n");
			usleep(100000);
			serial_dev_ack_nack_task(CUR_DATA_INIT_REQUEST,CUR_DATA_INIT_REQUEST_LEN,AUTO_TASK_OK);		
		}
		else if((com_buffer[0] == CUR_AUTO_CAL_REQUEST) && (com_cnt == CUR_AUTO_CAL_REQUEST_LEN))	//231206 Modify
		{
			cur_auto_cal_task();	
		}	
		else if((com_buffer[0] == CUR_DATA_SAVE_REQUEST) && (com_cnt == CUR_DATA_SAVE_REQUEST_LEN))	//231206 Modify
		{	
			cur_cal_offset_save_task();				
		}
		else if((com_buffer[0] == CUR_CHECK_REQUEST) && (com_cnt == CURCHECK_REQUEST_LEN))	//231206 Modify
		{
			short vol = 0;
			unsigned char ch = 0;
			unsigned char result = 0;
			ch = com_buffer[1]; 
			vol = (com_buffer[3] << 8) || com_buffer[4];  
			//if(ch == EX_CAL_AVDD)	result = cur_sensing_compare_task(ch, com_buffer[2], vol, 1);
			//else	if(ch == EX_CAL_AVDD)	result = cur_sensing_compare_task(ch, com_buffer[2], 1, vol);
			power_off(); 
			cal_end_task();	
			if(result != AUTO_TASK_OK)	serial_dev_ack_nack_task(CUR_CHECK_REQUEST,CURCHECK_REQUEST_LEN,AUTO_TASK_NG);	
			else	serial_dev_ack_nack_task(CUR_CHECK_REQUEST,CURCHECK_REQUEST_LEN,AUTO_TASK_OK);	 							
		}	
		else if((com_buffer[0] == RES_SELECT_REQUEST) && (com_cnt == RES_SELECT_REQUEST_LEN))	//231206 Modify
		{
			res_select_task(com_buffer[1],com_buffer[2]);
		}	
		else if((com_buffer[0] == RELAY_INIT_REQUEST) && (com_cnt == RELAY_INIT_REQUEST_LEN))	//231206 Modify
		{
			cal_end_task();
			if(pprf)	printf("EP975 RELAY_READY_MODE\r\n");	
			serial_dev_ack_nack_task(RELAY_INIT_REQUEST,RELAY_INIT_REQUEST_LEN,AUTO_TASK_OK);	 	
		}
		else if((com_buffer[0] == MEASUREMENT_RES_REQUEST) && (com_cnt == MEASUREMENT_RES_REQUEST_LEN))	//231206 Modify
		{
			measurement_res_select_task(com_buffer[1]);
		}	
		else if((com_buffer[0] == KEITHLEY_CONNECTION_REQUEST) && (com_cnt == KEITHLEY_CONNECTION_REQUEST_LEN))	//231206 Modify
		{
			output_to_keithley_connect_task(com_buffer[1]);	
		}	
		else if((com_buffer[0] == COMMAND_SETTING_REQUEST) && (com_cnt == COMMAND_SETTING_REQUEST_LEN))	//231206 Modify
		{
			printf("COMMAND_SETTING_REQUEST\r\n");
			memset(&category, 0, sizeof(category));
			memset(&es975_state, 0, sizeof(es975_state));
			memset(&categoty_state, 0, sizeof(categoty_state));
			memcpy(&category,&com_buffer[1],sizeof(category));
			serial_dev_ack_nack_task(COMMAND_SETTING_REQUEST,0x02,AUTO_TASK_OK);	
		}
		//else if((com_buffer[0] == COMMAND_START_REQUEST) && (com_cnt == COMMAND_START_REQUEST_LEN))	//231206 Modify
		else if((com_buffer[0] == COMMAND_START_REQUEST))	//231206 Modify
		{
			unsigned char current_state=0;
			if(com_buffer[1] == DAC_ADC_AUTO_START)
			{
				ts_dac_adc_auto_task_set(total_status,DAC_ADC_AUTO_START);  	
				if((es975_state.state_index == CATEGORY_STATE_IDEL))
				{
					serial_dev_ack_nack_task(COMMAND_START_REQUEST,COMMAND_START_REQUEST_LEN,AUTO_TASK_OK);	
					//category_task();	
				}
				else	serial_dev_ack_nack_task(COMMAND_START_REQUEST,COMMAND_START_REQUEST_LEN,AUTO_TASK_NG);
			}
			else if(com_buffer[1] == DAC_ADC_AUTO_STOP)
			{
				ts_dac_adc_auto_task_set(total_status,DAC_ADC_AUTO_STOP);
			}		 
		}
		else if((com_buffer[0] == ES975_STATE_REQUEST) && (com_cnt == ES975_STATE_REQUEST_LEN))	//231206 Modify
		{
			unsigned char category_start_flag = 0xff;
			ts_dac_adc_auto_task_get(total_status,&category_start_flag);    
			if(category_start_flag == DAC_ADC_AUTO_START)	
			{		
				es975_state_ch = 0;
				es975_state_ch = com_buffer[1];
				com_task_ack(ES975_STATE_REQUEST); 
			}
		}															
		else if((com_buffer[0] == RESET_REQUEST) && (com_cnt == RESET_REQUEST_LEN) && (pg_on_flag == PG_OFF))
		{
			printf("*******************************************REBOOT************************************************************\r\n");
			system("reboot");			
		}	
		else ("COMMAND NOT FOUND\r\n");		

		/*ACK:
		uart_ack();
		return;
		NACK:
		uart_nack();
		return;	
		DEFLAT:
		return;*/		
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

void pg_reboot_ack(void)
{
    char data[8] = {0};
    data[0] = 0x02;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x02;
    data[4] = REBOOT_ALARM;
    data[5] = 0x00;
    data[6] = 0xC0;
    data[7] = 0x03;	
    sdcd_serial_write(8,data);
}

void rm_adc_init_ng_ack(void)
{
    char data[8] = {0};
    data[0] = 0x02;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x02;
    data[4] = RM_ADC_INIT_NG_ALARM;
    data[5] = 0x00;
    data[6] = 0xC7;
    data[7] = 0x03;	
    sdcd_serial_write(8,data);
}

void uart_index_ack(unsigned char index)
{
    char data[8] = {0};
	int i = 0;
	unsigned short checksum=0;
    data[0] = 0x02;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x02;
    data[4] = 0x06;
    data[5] = index;
	for(i=1; i<6; i++)	checksum ^= *(data+i);
    data[6] = checksum;
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
	int l = 0;	
	char str[COM_BUFFER_SIZE]={0};
	unsigned short checksum=0;
	unsigned short tx_len = 0;	


	if(cmd == RESISTANCE_REQUEST_ACK)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = 0x90;
		str[tx_len++] = 0x80;
		str[tx_len++] = RESISTANCE_REQUEST_ACK_LEN;

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
		str[tx_len++] = 0x92;
		str[tx_len++] = 0x80;		
		str[tx_len++] = SENSING_DATA_REQUEST_ACK_LEN;
		for(j = 0 ; j < 40 ; j++)
		{
			str[tx_len++] = (adc_sensing_value[j]>>8)&0xff;			
			str[tx_len++] = adc_sensing_value[j]&0xff;			
		}
		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03; 
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}
	else if(cmd == SENSING_DATA_CUR_4BYTE_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = SENSING_DATA_CUR_4BYTE_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = SENSING_DATA_CUR_4BYTE_REQUEST_ACK_LEN;
		for(j = 0 ; j < 22 ; j++)
		{
			str[tx_len++] = (adc_sensing_value[j]>>8)&0xff;			
			str[tx_len++] = adc_sensing_value[j]&0xff;			
		}

		str[tx_len++] = (cur_sensing_value[0]>>24)&0xff;
		str[tx_len++] = (cur_sensing_value[0]>>16)&0xff;
		str[tx_len++] = (cur_sensing_value[0]>>8)&0xff;
		str[tx_len++] = (cur_sensing_value[0])&0xff;

		for(j = 23 ; j < 32 ; j++)
		{
			str[tx_len++] = (adc_sensing_value[j]>>8)&0xff;			
			str[tx_len++] = adc_sensing_value[j]&0xff;			
		}

		str[tx_len++] = (cur_sensing_value[1]>>24)&0xff;
		str[tx_len++] = (cur_sensing_value[1]>>16)&0xff;
		str[tx_len++] = (cur_sensing_value[1]>>8)&0xff;
		str[tx_len++] = (cur_sensing_value[1])&0xff;		

		for(j = 33 ; j < 40 ; j++)
		{
			str[tx_len++] = (adc_sensing_value[j]>>8)&0xff;			
			str[tx_len++] = adc_sensing_value[j]&0xff;			
		}			



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
		str[tx_len++] = pattern_generator->PG_STATUS & 0x000000ff;
		for(i=1; i<tx_len; i++) checksum ^= *(str+i);
		str[tx_len++] = checksum;	
		str[tx_len++] = 0x03;	
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));		
	}
	else if(cmd == SHORT_CHECK_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = SHORT_CHECK_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = SENSING_DATA_REQUEST_ACK_LEN;
		for(j = 0 ; j < 40 ; j++)
		{
			str[tx_len++] = (adc_sensing_value[j]>>8)&0xff;			
			str[tx_len++] = adc_sensing_value[j]&0xff;			
		}

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}	
	else if(cmd == AVDD_ELVSS_VOLTAGE_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = AVDD_ELVSS_VOLTAGE_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = AVDD_ELVSS_VOLTAGE_REQUEST_ACK_LEN;

		/*str[tx_len++] = (adc_sensing_value[SEN_AVDD]>>8)&0xff;	
		str[tx_len++] = adc_sensing_value[SEN_AVDD]&0xff;	
		str[tx_len++] = (adc_sensing_value[SEN_ELVSS]>>8)&0xff;		
		str[tx_len++] = adc_sensing_value[SEN_ELVSS]&0xff;*/	

		str[tx_len++] = (avdd_vol_2byte>>8)&0xff;	
		str[tx_len++] = avdd_vol_2byte&0xff;	
		str[tx_len++] = (elvss_vol_2byte>>8)&0xff;		
		str[tx_len++] = elvss_vol_2byte&0xff;					

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}	
	else if(cmd == AVDD_ELVSS_CURRENT_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = AVDD_ELVSS_CURRENT_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = AVDD_ELVSS_CURRENT_REQUEST_ACK_LEN;

		str[tx_len++] = (adc_sensing_value[SEN_ELIDD]>>8)&0xff;			
		str[tx_len++] = adc_sensing_value[SEN_ELIDD]&0xff;		
		str[tx_len++] = (adc_sensing_value[SEN_ELISS]>>8)&0xff;			
		str[tx_len++] = adc_sensing_value[SEN_ELISS]&0xff;				

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}
	else if(cmd == ERROR_DATA_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = ERROR_DATA_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = ERROR_DATA_REQUEST_ACK_LEN;

		str[tx_len++] = error_data.i2c_communication&0xff;	
		str[tx_len++] = error_data.i2c_read&0xff;			
		str[tx_len++] = error_data.avdd_cur&0xff;		
		str[tx_len++] = error_data.elvss_cur&0xff;			
		str[tx_len++] = error_data.ocp&0xff;
		str[tx_len++] = error_data.vol_cur_adc&0xff;
		str[tx_len++] = error_data.resistance_adc&0xff;							

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}
	else if(cmd == CURRENT_MODEL_INDEX_REQUEST)		//230929 Modify
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = CURRENT_MODEL_INDEX_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = model_name_size+CURRENT_INDEX_SIZE;
		str[tx_len++] = pat_index;
		if(cprf)	printf("pat_index = %d / size = %d\r\n", pat_index, model_name_size+CURRENT_INDEX_SIZE);		
		for(l = 0 ; l< model_name_size ; l++)
		{
			str[tx_len++] = model_name[l];	
			if(cprf)	printf("model_name[%d] = %c / %x\r\n", l , model_name[l],model_name[l]);
		}
		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));		
	}
	else if(cmd == ES975_STATE_REQUEST)		//231206 Modify
	{
		ES975_STATE state_result;
		memset(&str, 0, sizeof(str));
		if(ring_q_peek(queue_cal_result,(char *)&state_result, sizeof(ES975_STATE))>0)
		{
			ring_q_flush(queue_cal_result,sizeof(ES975_STATE));
		}		
		//es975_state.state_index = categoty_state[es975_state_ch];
		//es975_state.ch_index = es975_state_ch; 
		str[tx_len++] = 0x02;
		str[tx_len++] = ES975_STATE_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = ES975_STATE_REQUEST_ACK_LEN;	
		memcpy(&str[tx_len],&state_result,sizeof(state_result));	
		tx_len += sizeof(state_result);
		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		//for(i = 0 ; i < tx_len ; i++)	printf("%x  ",	str[i]);
		//printf("\r\n");
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));

		if((state_result.state_index == CATEGORY_STATE_ALLEND) || (state_result.state_index == CATEGORY_STATE_KEITHLEY_COMMUNICATION_FAIL) || (state_result.state_index == CATEGORY_STATE_NG))	ts_dac_adc_auto_task_set(total_status,DAC_ADC_AUTO_STOP);
		
		/*if(es975_state.state_index != CATEGORY_STATE_EXECUTE)
		{
			categoty_state[es975_state_ch] = CATEGORY_STATE_IDEL; // STATE(NG, KEITHLEY_COMMUNICATION_FAIL, END, ALLEND) Convert to Ready after Send	
			ts_dac_adc_auto_task_set(total_status,TASK_END);
		}*/	
	}						
}

void bist_ack(unsigned char mode, unsigned char result)
{
	int i = 0;
	int j = 0;
	char str[COM_BUFFER_SIZE]={0};
	unsigned short checksum=0;
	unsigned short tx_len = 0;

	str[tx_len++] = 0x02;
	str[tx_len++] = 0x26;
	str[tx_len++] = 0x80;		
	str[tx_len++] = 0x02;
	str[tx_len++] = mode;	
	str[tx_len++] = result;
	for(i=1; i<tx_len; i++) checksum ^= *(str+i);
	str[tx_len++] = checksum;	
	str[tx_len++] = 0x03;	
	sdcd_serial_write(tx_len, str);
	memset(&str, 0, sizeof(str));		
}

void com_sensing_task_ack(unsigned char cmd, unsigned char *avdd_cur, unsigned char *elvss_cur)
{	
	int i = 0;
	int j = 0;
	char str[COM_BUFFER_SIZE]={0};
	unsigned short checksum=0;
	unsigned short tx_len = 0;	

	if(cmd == ELVSS_4BYTE_CURRENT_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = ELVSS_4BYTE_CURRENT_REQUEST;
		str[tx_len++] = 0x80;
		str[tx_len++] = ELVSS_4BYTE_CURRENT_REQUEST_ACK_LEN;

		str[tx_len++] = *(elvss_cur+3)&0xff;
		str[tx_len++] = *(elvss_cur+2)&0xff;
		str[tx_len++] = *(elvss_cur+1)&0xff;
		str[tx_len++] = *(elvss_cur)&0xff;

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03;
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));		
	}
	else if(cmd == AVDD_ELVSS_4BYTE_CURRENT_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = AVDD_ELVSS_4BYTE_CURRENT_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = AVDD_ELVSS_4BYTE_CURRENT_REQUEST_ACK_LEN;

		str[tx_len++] = *(avdd_cur+3)&0xff;
		str[tx_len++] = *(avdd_cur+2)&0xff;
		str[tx_len++] = *(avdd_cur+1)&0xff;
		str[tx_len++] = *(avdd_cur)&0xff;
		str[tx_len++] = *(elvss_cur+3)&0xff;
		str[tx_len++] = *(elvss_cur+2)&0xff;
		str[tx_len++] = *(elvss_cur+1)&0xff;
		str[tx_len++] = *(elvss_cur)&0xff;	

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03; 
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}
	else if(cmd == AVDD_ELVSS_CURRENT_REQUEST)
	{
		str[tx_len++] = 0x02;
		str[tx_len++] = AVDD_ELVSS_CURRENT_REQUEST;
		str[tx_len++] = 0x80;		
		str[tx_len++] = AVDD_ELVSS_CURRENT_REQUEST_ACK_LEN;


		str[tx_len++] = *(avdd_cur+1)&0xff;
		str[tx_len++] = *(avdd_cur)&0xff;
		str[tx_len++] = *(elvss_cur+1)&0xff;
		str[tx_len++] = *(elvss_cur)&0xff;	

		for(i=1; i<tx_len; i++)	checksum ^= *(str+i);	
		str[tx_len++] = checksum;
		str[tx_len++] = 0x03; 
		sdcd_serial_write(tx_len, str);
		memset(&str, 0, sizeof(str));						
	}		
}

void ext_com_task_ack(unsigned char cmd, unsigned char len, unsigned char *data)
{
	char str[100];
	int i = 0;
	unsigned short checksum=0;
	unsigned short tx_len = 0;	

	memset(&str, 0, sizeof(str));
	str[tx_len++] = 0x02;
	str[tx_len++] = cmd;
	str[tx_len++] = 0x80;	
	str[tx_len++] = len;
	for(i = 0 ; i < len ; i++)	str[tx_len++] = *(data+i);		
	for(i=1; i<tx_len; i++)	checksum ^= *(str+i);
	str[tx_len++] = (uint8_t)(0xff&checksum);
	str[tx_len++] = 0x03;
	sdcd_serial_write(tx_len, str);
	memset(&str, 0, sizeof(str));			
}
