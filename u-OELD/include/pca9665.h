#ifndef __PCA9665__
#define __PCA9665__

extern int i2c_fd;
extern void i2c_init(void);
extern int i2c_set_frequency(int frequency);
extern int i2c_read(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *read_buffer, int read_byte);
extern int i2c_write(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *write_buffer, int write_byte);
int i2c_write_test(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *write_buffer, int write_byte);
int i2c_read_test(unsigned char SLAVE_ADDRESS, unsigned short REG_ADDRESS, unsigned char *read_buffer, int read_byte);

#endif
