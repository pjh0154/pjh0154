#include "../include/pca9665.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/mman.h>
#include "../include/ep.h"

//#define I2C_DEV_FILE "/dev/i2c-0"
#define I2C_DEV_FILE "/dev/i2c-2"
#define PCA9665_BASE_ADDR 0x60000000

#define I2C_PCA_MODE_STD      0x00
#define I2C_PCA_MODE_FAST      0x01
#define I2C_PCA_MODE_FASTP      0x02
#define I2C_PCA_MODE_TURBO      0x03
#define I2C_PCA_OSC_PER         3

int i2c_fd = -1;
unsigned char *pca9665_ptr = NULL;
extern int cprf;
extern int eprf;
void i2c_init(void);
int i2c_set_frequency(int frequency);
int i2c_read(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *read_buffer, int read_byte);
int i2c_write(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *write_buffer, int write_byte);

void i2c_init(void)
{
    int mem_fd;
    void *map_base;
    unsigned char I2C_TO;

    mem_fd = open("/dev/mem", O_RDWR);
    if(mem_fd < 0){
        printf("/dev/mem open error (pca9665_ptr)\r\n");
        return;
    }
    pca9665_ptr = (unsigned char *)mmap((void *)0, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, PCA9665_BASE_ADDR);
    if(pca9665_ptr == NULL){
        printf("pca9665_ptr mmap setting fail\r\n");
        close(mem_fd);
        return;
    }
    close(mem_fd);

    i2c_fd = open(I2C_DEV_FILE, O_RDWR);
    if(i2c_fd < 0){
        printf("I2C(PCA9665) Open Fail\r\n");
        i2c_fd = -1;
    }
    else printf("I2C(PCA9665) Open OK %d\r\n", i2c_fd);
}

int i2c_set_frequency(int frequency)
{
    int pca_clock = frequency;
    int clock;
    int mode;
    int raise_fall_time;
    int tlow, thi;
    int min_tlow, min_thi;

    if(cprf) printf("frequency = [%d]Hz\r\n", pca_clock);
    if(pca_clock > 1265800){
        printf("I2C clock speed too high, set 1265800[Hz]\r\n");
        pca_clock = 1265800;
    }
    if(pca_clock < 60300){
        printf("I2C clock speed too low, set 60300[Hz]\r\n");
        pca_clock = 60300;
        pca_clock = I2C_PCA_MODE_TURBO;
    }
    
    clock = pca_clock / 100;
    if (pca_clock > 1000000) {
        mode = I2C_PCA_MODE_TURBO;
        min_tlow = 14;
        min_thi = 5;
        raise_fall_time = 22;
    } else if (pca_clock > 400000) {
        mode = I2C_PCA_MODE_FASTP;
        min_tlow = 17;
        min_thi = 9;
        raise_fall_time = 22;
    } else if (pca_clock > 100000) {
        mode = I2C_PCA_MODE_FAST;
        min_tlow = 44;
        min_thi = 20;
        raise_fall_time = 58;
    } else {
        mode = I2C_PCA_MODE_STD;
        min_tlow = 157;
        min_thi = 134;
        raise_fall_time = 127;
    }
    
    if (clock < 648) {
        tlow = 255;
        thi = 1000000 - clock * raise_fall_time;
        thi /= (I2C_PCA_OSC_PER * clock) - tlow;
    } else {
        tlow = (1000000 - clock * raise_fall_time) * min_tlow;
        tlow /= I2C_PCA_OSC_PER * clock * (min_thi + min_tlow);
        thi = tlow * min_thi / min_tlow;
    }
    *(pca9665_ptr + 0) = 6; //INDPTR REG WRITE -> I2CMODE REG
    *(pca9665_ptr + 2) = mode;
    //printf("[I2CMODE] mode = [%x] %x\r\n", *(pca9665_ptr + 2), mode);

    *(pca9665_ptr + 0) = 3; //INDPTR REG WRITE -> I2CSCLH REG
    *(pca9665_ptr + 2) = thi;
    //printf("[I2CSCLH] thi = [%x] %x\r\n", *(pca9665_ptr + 2), thi);

    *(pca9665_ptr + 0) = 2; //INDPTR REG WRITE -> I2CSCLL REG
    *(pca9665_ptr + 2) = tlow;
    //printf("[I2CSCLL] tlow = [%x] %x\r\n", *(pca9665_ptr + 2), tlow);
}

int i2c_read(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *read_buffer, int read_byte)
{
    unsigned char reg_addr[2];
    if(i2c_fd < 0){printf("I2C Device Not Init\r\n");return -1;}

    ioctl(i2c_fd, I2C_SLAVE, SLAVE_ADDRESS);
    reg_addr[0] = (REG_ADDRESS & 0xFF00) >> 8;
    reg_addr[1] = REG_ADDRESS & 0x00FF;
    if((write(i2c_fd, reg_addr, 2)) < 0){
        if(eprf)    printf("REG_ADDRESS Write Fail\r\n");   //231013 Modify
        return -1;
    }

    if((read(i2c_fd, read_buffer, read_byte)) < 0){
        if(eprf)    printf("I2C(PCA9665) Read Fail\r\n");   //231013 Modify
        return -1;
    }    
    return 0;
}

int i2c_write(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *write_buffer, int write_byte)
{
    unsigned char tx_buffer[I2C_MAN_LEN];
    int i = 0;
    int tx_size;

    if(i2c_fd < 0){printf("I2C Device Not Init\r\n");return -1;}
    if(write_buffer != NULL)    //230929 Modify	
    {
        ioctl(i2c_fd, I2C_SLAVE_FORCE, SLAVE_ADDRESS);
        tx_buffer[0] = (REG_ADDRESS & 0xFF00) >> 8;
        tx_buffer[1] = REG_ADDRESS & 0x00FF;
        memmove(&tx_buffer[2], write_buffer, write_byte);
        //memcpy(&tx_buffer[2], write_buffer, write_byte);
        if(cprf)
        {
            printf("SLAVE_ADDRESS = %x\r\n", SLAVE_ADDRESS);
            printf("REG_ADDRESS = %x\r\n", REG_ADDRESS);
            printf("write_byte = %x\r\n", write_byte);
            for(i = 0 ; i < write_byte ; i++)   printf("tx_buffer[%d] = %x\r\n", i, tx_buffer[2+i]);            
        }

        tx_size = write(i2c_fd, tx_buffer, write_byte + 2);

        if(tx_size != (write_byte + 2)){
            //printf("I2C(PCA9665) Write Fail\r\n");
            //if(cprf)
            if(eprf)    //231013 Modify
            {
                printf("I2C(PCA9665) Write Fail\r\n");
                printf("SLAVE_ADDRESS = %x\r\n", SLAVE_ADDRESS);
                printf("REG_ADDRESS = %x\r\n", REG_ADDRESS);
                printf("write_byte = %x\r\n", write_byte);   
            }     
            return -1;
        }
        return 0;
    }
    else    return -1; 
}



