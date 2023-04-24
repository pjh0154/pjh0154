#ifndef gpio_H
#define gpio_H

#include "global.h"
#include "ep.h"

#define R_ON_OFF_H 1<<0
#define R_ON_OFF_L 0<<0

#define R_SEL1_H 1<<1
#define R_SEL1_L 0<<1

#define R_SEL1K_H 1<<2
#define R_SEL1K_L 0<<2

#define R_SOURCE_ON_OFF_H 1<<3
#define R_SOURCE_ON_OFF_L 0<<3

#define L_ON_OFF_H 1<<4
#define L_ON_OFF_L 0<<4

#define L_SEL1_H 1<<5
#define L_SEL1_L 0<<5

#define L_SEL1K_H 1<<6
#define L_SEL1K_L 0<<6

#define L_SOURCE_ON_OFF_H 1<<7
#define L_SOURCE_ON_OFF_L 0<<7

#define I2C_nRESET_H 1<<8
#define I2C_nRESET_L 0<<8

#define I2C_nINT_H 1<<9
#define I2C_nINT_L 0<<9

#define gpio_init_value 0x00000000

void gpio_reg_init(void);
void logic_gpio_ctl(char *data, char stat);
#endif