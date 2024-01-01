#ifndef GPIO_DEV_H
#define GPIO_DEV_H

/*#define GPIO_DIR_IN     0
#define GPIO_DIR_OUT    1

typedef struct
{
    unsigned int PinNum;
    unsigned char Direction;
    unsigned char Default;
} Pin;


int gpio_export(unsigned gpio);
int gpio_unexport(unsigned gpio);
int gpio_dir_out(unsigned gpio);
int gpio_dir_in(unsigned gpio);
int gpio_set_value(unsigned gpio, unsigned value);
int gpio_get_value(unsigned gpio);*/

#define GPIO_LOW        0
#define GPIO_HIGH       1

void gpiops_init(void);
int gpiops_export(unsigned int mio_num);
int gpiops_unexport(unsigned int mio_num);
int gpiops_dir_out(unsigned int mio_num);
int gpiops_dir_in(unsigned int mio_num);
int gpiops_set_value(unsigned int mio_num, unsigned int value);
int gpiops_get_value(unsigned int mio_num);


#endif


