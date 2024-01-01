#include "../include/signal_power.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <math.h>

#define PVSIG_DEVICE_NAME "/dev/ttyUL3"
#define NVSIG_DEVICE_NAME "/dev/ttyUL2"

#define POTENTIAL_CAL(X) ((-9.52381*X)+290.7143)

static int pvsig_fd = -1;
static int nvsig_fd = -1;
int ex_fd = -1;

void signal_power_device_init(void);
void pvsig_rs232_send(char *tx_buffer, int tx_len);
void nvsig_rs232_send(char *tx_buffer, int tx_len);
void pvsig_rs232_test(void);
void nvsig_rs232_test(void);
void signal_power_rs232_rx_task(void);
void pvsig_onoff(bool on);
void nvsig_onoff(bool on);
void pvsig_voltage_set(double voltage);
void nvsig_voltage_set(double voltage);
void sig_gpio_init(void);

void signal_power_device_init(void)
{
    struct termios newtio;

    pvsig_fd = open(PVSIG_DEVICE_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(pvsig_fd < 0){
        printf("PVSIG "PVSIG_DEVICE_NAME" Open Fail\r\n");
        return;
    }

    nvsig_fd = open(NVSIG_DEVICE_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(nvsig_fd < 0){
        printf("NVSIG "PVSIG_DEVICE_NAME" Open Fail\r\n");
        close(pvsig_fd);
        pvsig_fd = -1;
        return;
    }

    memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag		= B9600 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag		= 0;         // parity none
	newtio.c_oflag		= 0;
	newtio.c_lflag		= 0;
	newtio.c_cc[VTIME]	= 0;
	newtio.c_cc[VMIN]	= 1;
    
	tcflush  (pvsig_fd, TCIFLUSH );
	tcsetattr(pvsig_fd, TCSANOW, &newtio );
   	fcntl(pvsig_fd, F_SETFL, FNDELAY);

    tcflush  (nvsig_fd, TCIFLUSH );
	tcsetattr(nvsig_fd, TCSANOW, &newtio );
   	fcntl(nvsig_fd, F_SETFL, FNDELAY);



    sig_gpio_init();
    pvsig_onoff(false);
    nvsig_onoff(false);
    printf("SIGNAL POWER INIT OK\r\n");
}

/*void signal_device_init(void)
{
    struct termios newtio;

    ex_fd = open(EXT_DEVICE_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(ex_fd < 0){      //231027 Modify
        printf("PVSIG "EXT_DEVICE_NAME" Open Fail\r\n");
        return;
    }

    memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag		= B115200 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag		= 0;         // parity none
	newtio.c_oflag		= 0;
	newtio.c_lflag		= 0;
	newtio.c_cc[VTIME]	= 0;
	newtio.c_cc[VMIN]	= 1;
    
	tcflush  (ex_fd, TCIFLUSH );
	tcsetattr(ex_fd, TCSANOW, &newtio );
   	fcntl(ex_fd, F_SETFL, FNDELAY);

    printf("EX_SIGNAL POWER INIT OK\r\n");
}*/

void pvsig_rs232_send(char *tx_buffer, int tx_len)
{
    char temp_tx_buffer[POWER_SET_MAX_LEN]; //230929 Modify
    int temp_tx_len = tx_len;
    int tx_empty_state = 0;
    int i = 0;
	int pvsig_start = 0;	//231015 Modify
	int pvsig_end = 0;	//231015 Modify
	int pvsig_time = POWER_SET_TIMEOUT;	//231015 Modify    

    if(tx_buffer != NULL)
    {
        memset(temp_tx_buffer, 0, sizeof(temp_tx_buffer));
        memcpy(temp_tx_buffer, tx_buffer, temp_tx_len);

        if(pvsig_fd < 0){
            printf("PVSIG Device Not Open\r\n");
            return;
        }
        else{
            write(pvsig_fd, temp_tx_buffer, temp_tx_len);
            pvsig_start = timeout_msclock();
            while(true){
                ioctl(pvsig_fd, TIOCSERGETLSR, &tx_empty_state);
                if(tx_empty_state) break;
                /*i++;                //230929 Modify
                if(i > 3000000)     //230929 Modify
                {
                    printf("PVSIG_RS232_ERROR\r\n");
                    system_error.pvssig_rs232 = ERROR_NG;
                    break;
                }*/
                usleep(1);    //231013 Modify 
			    pvsig_end = timeout_msclock() - pvsig_start;    //231015 Modify
			    if(pvsig_end >= pvsig_time) //231015 Modify 
                {
                    printf("PVSIG_RS232_ERROR\r\n");
                    system_error.pvssig_rs232 = ERROR_NG;                   
                    break;	       
                }         
            }
        }
    }
}

void nvsig_rs232_send(char *tx_buffer, int tx_len)
{
    char temp_tx_buffer[POWER_SET_MAX_LEN];     //230929 Modify
    int temp_tx_len = tx_len;
    int tx_empty_state = 0;
    int i = 0;
	int nvsig_start = 0;	//231015 Modify
	int nvsig_end = 0;	//231015 Modify
	int nvsig_time = POWER_SET_TIMEOUT;	//231015 Modify     

    if(tx_buffer != NULL)
    {    
        memset(temp_tx_buffer, 0, sizeof(temp_tx_buffer));
        memcpy(temp_tx_buffer, tx_buffer, temp_tx_len);

        if(nvsig_fd < 0){
            printf("NVSIG Device Not Open\r\n");
            return;
        }
        else{
            write(nvsig_fd, temp_tx_buffer, temp_tx_len);        
            nvsig_start = timeout_msclock();
            while(true){
                ioctl(nvsig_fd, TIOCSERGETLSR, &tx_empty_state);
                if(tx_empty_state) break;
                /*i++;            //230929 Modify
                if(i > 3000000) //230929 Modify 
                {
                    printf("NVSIG_RS232_ERROR\r\n");
                    system_error.nvssig_rs232 = ERROR_NG;
                    break;
                }*/
                usleep(1);    //231013 Modify 
			    nvsig_end = timeout_msclock() - nvsig_start;    //231015 Modify
			    if(nvsig_end >= nvsig_time) //231015 Modify 
                {
                    printf("NVSIG_RS232_ERROR\r\n");
                    system_error.nvssig_rs232 = ERROR_NG;                 
                    break;	       
                }                 
            }
        }
    }
}

/*void ex_rs232_send(char *tx_buffer, int tx_len)
{
    char temp_tx_buffer[EX_RS232_MAX_LEN];  //230929 Modify
    int temp_tx_len = tx_len;
    int tx_empty_state = 0;
    int i = 0;    
	int ex_start = 0;	//231015 Modify
	int ex_end = 0;	//231015 Modify
	int ex_time = POWER_SET_TIMEOUT;	//231015 Modify       

    if(tx_buffer != NULL)
    {
        memset(temp_tx_buffer, 0, sizeof(temp_tx_buffer));
        memcpy(temp_tx_buffer, tx_buffer, temp_tx_len);

        if(ex_fd < 0){
            printf("PVSIG Device Not Open\r\n");
            return;
        }
        else{
            write(ex_fd, temp_tx_buffer, temp_tx_len);
            ex_start = timeout_msclock();
            while(true){
                ioctl(ex_fd, TIOCSERGETLSR, &tx_empty_state);
                if(tx_empty_state) break;
                usleep(1);    //231013 Modify     
			    ex_end = timeout_msclock() - ex_start;    //231015 Modify
			    if(ex_end >= ex_time) //231015 Modify 
                {
                    printf("EX_RS232_ERROR\r\n");
                    //system_error.nvssig_rs232 = ERROR_NG;                
                    break;	       
                }                              
            }
        }
    }
}*/

void pvsig_rs232_test(void)
{
    pvsig_rs232_send("VP", 2);
}

void nvsig_rs232_test(void)
{
    nvsig_rs232_send("VP", 2);
}

void signal_power_rs232_rx_task(void)
{
    int read_count;
    char pvsig_buffer[32] = {0};
    char nvsig_buffer[32] = {0};
    int i = 0;

    //thread use

    while(true){
        read_count = read(pvsig_fd, pvsig_buffer, sizeof(pvsig_buffer));
        if(read_count > 0){
            for(int i=0; i<read_count; i++) printf("PVSIG RX : 0x%02x , %c\r\n", pvsig_buffer[i], pvsig_buffer[i]);
        }
        read_count = read(nvsig_fd, nvsig_buffer, sizeof(nvsig_buffer));
        if(read_count > 0){
            for(int i=0; i<read_count; i++) printf("NVSIG RX : 0x%02x , %c\r\n", nvsig_buffer[i], nvsig_buffer[i]);
        }
        usleep(1000);
    }
}

void sig_gpio_init(void)
{
    char tx_data[6];

    tx_data[0] = 0x57;
    tx_data[1] = 0x02;
    tx_data[2] = 0xaa;
    tx_data[3] = 0x03;
    tx_data[4] = 0xaa;
    tx_data[5] = 0x50;

    pvsig_rs232_send(tx_data, sizeof(tx_data));
    nvsig_rs232_send(tx_data, sizeof(tx_data));
}

void pvsig_onoff(bool on)
{
    char tx_data[3] = {0x4f,(on == true) ? 0xff : 0x00, 0x50};
    pvsig_rs232_send(tx_data, sizeof(tx_data));
}

void nvsig_onoff(bool on)
{
    char tx_data[3] = {0x4f,(on == true) ? 0xff : 0x00, 0x50};
    nvsig_rs232_send(tx_data, sizeof(tx_data));
}

void pvsig_voltage_set(double voltage)
{
    char tx_data[6] = {0};

    if(pvsig_fd < 0){
        printf("PVSIG Device Not Open\r\n");
        return;
    }
    else{
        unsigned char temp_uchar;
        double temp_voltage = fabs(voltage);

        temp_uchar = (unsigned char)temp_voltage; //이거 대신 계산식??

        if(temp_uchar < 5) temp_uchar = 5;
        tx_data[0] = 0x53;
        tx_data[1] = 0x5E;
        tx_data[2] = 0x02;
        tx_data[3] = 0x00;
        tx_data[4] = temp_uchar;
        tx_data[5] = 0x50;
        pvsig_rs232_send(tx_data, sizeof(tx_data));
    }
}

void nvsig_voltage_set(double voltage)
{
    char tx_data[6] = {0};

    if(nvsig_fd < 0){
        printf("NVSIG Device Not Open\r\n");
        return;
    }
    else{
        unsigned char temp_uchar;
        double temp_voltage = fabs(voltage);

        temp_uchar = (unsigned char)temp_voltage; //이거 대신 계산식??

        if(temp_uchar < 5) temp_uchar = 5;
        tx_data[0] = 0x53;
        tx_data[1] = 0x5E;
        tx_data[2] = 0x02;
        tx_data[3] = 0x00;
        tx_data[4] = temp_uchar;
        tx_data[5] = 0x50;
        nvsig_rs232_send(tx_data, sizeof(tx_data));
    }
}
