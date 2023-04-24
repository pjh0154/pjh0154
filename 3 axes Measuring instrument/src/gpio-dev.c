/*#include "../include/gpio-dev.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
 
int gpio_export(unsigned gpio)
{
	int fd, len;
	char buf[11];

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}
 
int gpio_unexport(unsigned gpio)
{
	int fd, len;
	char buf[11];
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}
 
int gpio_dir(unsigned gpio, unsigned dir)
{
	int fd, len;
	char buf[60];
 
	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);
 	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (dir == GPIO_DIR_OUT)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}
 
int gpio_dir_out(unsigned gpio)
{
	return gpio_dir(gpio, GPIO_DIR_OUT);
}
 
int gpio_dir_in(unsigned gpio)
{
	return gpio_dir(gpio, GPIO_DIR_IN);
}
 
int gpio_set_value(unsigned gpio, unsigned value)
{
	int fd, len;
	char buf[60];

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
 	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);
 
	close(fd);
	return 0;
}
 
int gpio_get_value(unsigned gpio)
{
	int fd, len;
	char buf[60];
 
	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
 	
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
 
 	memset( buf, 0, sizeof(buf) );
 	sync();
	len = read(fd, buf, 1);
 
	close(fd);

	if( buf[0] == '1' )
		return 1;
	else
	if( buf[0] == '0' )
		return 0;

	return 0;
}*/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../include/gpio-dev.h"

#define GPIO_DIR_IN		0
#define GPIO_DIR_OUT	1
static int gpiops_base_number = -1;

void gpiops_init(void)
{
	FILE *fp = NULL;
	DIR *dir_ptr = NULL;
	struct dirent *file = NULL;
	int i;
	int temp_base_number;
	char temp_str[128];

	if((dir_ptr = opendir("/sys/class/gpio")) == NULL){
		printf("gpio directory find fail\r\n");
		return;
	}

	while((file = readdir(dir_ptr)) != NULL){
		if(strstr(file->d_name, "gpiochip") != NULL){
			temp_base_number = 0;
			for(i=0; i<strlen(file->d_name); i++){
				if((file->d_name[i] >= '0') && (file->d_name[i] <= '9'))
                    temp_base_number = (temp_base_number * 10) + (file->d_name[i] - '0');
			}
			memset(temp_str, 0, sizeof(temp_str));
			sprintf(temp_str, "/sys/class/gpio/%s/label", file->d_name);
			if(fp = fopen(temp_str, "r")){
				memset(temp_str, 0, sizeof(temp_str));
				fgets(temp_str, sizeof(temp_str), fp);
				if(!strncmp(temp_str, "zynq_gpio", 9)){
					gpiops_base_number = temp_base_number;
					fclose(fp);
					closedir(dir_ptr);
					printf("gpiops init ok, base num = %d\r\n", gpiops_base_number);
					return;
				}
				fclose(fp);
			}
		}
	}
	closedir(dir_ptr);
	printf("gpiops init fail\r\n");
}

int gpiops_export(unsigned int mio_num)
{
	int fd, len;
	char buf[11];

	if(gpiops_base_number < 0) return -1;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpiops_base_number + mio_num);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}
 
int gpiops_unexport(unsigned int mio_num)
{
	int fd, len;
	char buf[11];
 
	if(gpiops_base_number < 0) return -1;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpiops_base_number + mio_num);
	write(fd, buf, len);
	close(fd);
	return 0;
}
 
static int gpiops_dir(unsigned int mio_num, unsigned int dir)
{
	int fd, len;
	char buf[60];
 
 	if(gpiops_base_number < 0) return -1;
	
	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpiops_base_number + mio_num);
 	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (dir == GPIO_DIR_OUT)
		write(fd, "out", 3);
	else
		write(fd, "in", 2);
 
	close(fd);
	return 0;
}
 
int gpiops_dir_out(unsigned int mio_num)
{
	return gpiops_dir(mio_num, GPIO_DIR_OUT);
}
 
int gpiops_dir_in(unsigned int mio_num)
{
	return gpiops_dir(mio_num, GPIO_DIR_IN);
}
 
int gpiops_set_value(unsigned int mio_num, unsigned int value)
{
	int fd, len;
	char buf[60];

	if(gpiops_base_number < 0) return -1;

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpiops_base_number + mio_num);
 	
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
 
	if (value)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);
 
	close(fd);
	return 0;
}
 
int gpiops_get_value(unsigned int mio_num)
{
	int fd, len;
	char buf[60];
 
 	if(gpiops_base_number < 0) return -1;

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpiops_base_number + mio_num);
 	
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/value");
		return fd;
	}
 
 	memset( buf, 0, sizeof(buf) );
 	sync();
	len = read(fd, buf, 1);
 
	close(fd);

	if( buf[0] == '1' )
		return 1;
	else
	if( buf[0] == '0' )
		return 0;

	return 0;
}
